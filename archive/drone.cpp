#include <drone.h>
#include <pca9685.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <thread>
#include <chrono>

#include <mpu6050.h>
#include <logger.h>
#include <config.h>
#include <socket.h>
#include <bmp390.h>
#include <reporter.h>

// #include <cmath>
#include <limits>
#include <math.h>
#include <filter.h>
#include <pid.h>
#include <cmath>
#include <mutex>
// #define nan std::numeric_limits<double>::quiet_NaN()
#define G 9.81

// static double nan = std::numeric_limits<double>::quiet_NaN();

static double mpu6050_data[6];
static double filtered_mpu6050_data[6];
static filter::filter mpu6050_filters[6];
static math::quarternion orientation;

static double dt;

static math::vector orientation_euler;
static math::vector position(0, 0, 0), velocity(0, 0, 0);

static double bmp390_data[3];
static double valt;
static double old_altitude;
static double initial_altitude;


static double throttle = 0.0;
static double motor_fl_spd, motor_fr_spd, motor_bl_spd, motor_br_spd;

static bool alive = true;

static int sensor_ref_rate, sensor_sleep_int;
static int upper_sensor_freq_cutoff;
static double lower_sensor_freq_cutoff;
static std::thread sensor_thread;
static int settle_length;
static double sensor_roll_pitch_tau;
static double sensor_g_tolerance, sensor_g_tolerance_sqrd;

static double upper_pressure_freq_cutoff, upper_vz_freq_cutoff;
static filter::filter pressure_filter;
static filter::filter vzfilter;
static double sensor_z_tau;

static int message_thread_ref_rate, message_sleep_int;
static std::thread message_thread;
static std::string socket_path;

static bool zero_flag = false;
static bool calib_flag = false;

static double trim;
static pid /* x_controller, y_controller, */ z_controller;
static pid roll_controller, pitch_controller, vyaw_controller;
static double thrust = 0;
static double yawthrust = 0;

static double cpu_usg=-1, battery=-1;
static bool cntrller_connected = false;

static double debug_vals[6] = {0, 0, 0, 0, 0, 0};

enum state {
    configuring = 0, ready = 1, calibrating = 2, idle = 3, init = 4, settling = 5, destroying = 6
};

static state curr_state = state::init;


void drone::set_ctrller_connected_flag(bool connected) {
    cntrller_connected = connected;
}

pid * drone::get_roll_controller(){
    return &roll_controller;
}

pid * drone::get_pitch_controller(){
    return &pitch_controller;
}

pid * drone::get_z_controller(){
    return &z_controller;
}

pid * drone::get_vyaw_controller(){
    return &vyaw_controller;
}

void clear_led(){
    pca9685::set_pwm_ms(LED_RUN_PIN, 0);
    pca9685::set_pwm_ms(LED_AUTO_PIN, 0);
    pca9685::set_pwm_ms(LED_ERROR_PIN, 0);
    pca9685::set_pwm_ms(LED_WHITE_PIN, 0);
    pca9685::set_pwm_ms(LED_BLUE_PIN, 0);
}
void drone::force_terminate(){
    clear_led();
    pca9685::set_pwm_ms(LED_RUN_PIN, 0);
    pca9685::set_pwm_ms(LED_ERROR_PIN, PWM_FULL-1);
    drone::set_all(0);
}


void terminate_handle(int signum){
    drone::force_terminate();
    exit(0);
}


void drone::init(){
    logger::info("Initializing drone.");
    // Setup pca9685
    pca9685::set_frequency(50);
    pca9685::init();
    pca9685::wake_up();
    
    // Attach handle for ctrl+c
    signal(SIGINT, terminate_handle);
    atexit(drone::force_terminate);

    //initial LED control
    clear_led();
    pca9685::set_pwm_ms(LED_RUN_PIN, PWM_FULL - 1);
}

void drone::arm(){

}

void setup_filters(){
    for(int i = 0; i < 3; i ++){
        mpu6050_filters[i] = filter::low_pass(sensor_ref_rate, upper_sensor_freq_cutoff);
    }
    for(int i = 3; i < 6; i ++){
        mpu6050_filters[i] = filter::none();
    }

    mpu6050_filters[5] = filter::low_pass(sensor_ref_rate, upper_sensor_freq_cutoff);

    pressure_filter = filter::low_pass(sensor_ref_rate, upper_pressure_freq_cutoff);
    vzfilter = filter::low_pass(sensor_ref_rate, upper_vz_freq_cutoff);
}


void load_pid_config(){
    config::load_file();
    
    z_controller.kP = config::get_config_dbl("pid.z.kP", 0.01);
    z_controller.kI = config::get_config_dbl("pid.z.kI", 0.00);
    z_controller.kD = config::get_config_dbl("pid.z.kD", 0.00);
    
    roll_controller.kP = config::get_config_dbl("pid.r.kP", 0.01);
    roll_controller.kI = config::get_config_dbl("pid.r.kI", 0.00);
    roll_controller.kD = config::get_config_dbl("pid.r.kD", 0.00);
    
    pitch_controller.kP = config::get_config_dbl("pid.p.kP", 0.01);
    pitch_controller.kI = config::get_config_dbl("pid.p.kI", 0.00);
    pitch_controller.kD = config::get_config_dbl("pid.p.kD", 0.00);
    
    vyaw_controller.kP = config::get_config_dbl("pid.vyaw.kP", 0.01);
    vyaw_controller.kI = config::get_config_dbl("pid.vyaw.kI", 0.00);
    vyaw_controller.kD = config::get_config_dbl("pid.vyaw.kD", 0.00);
    
    trim = config::get_config_dbl("pid.trim", 0.0);

    config::write_to_file();

    logger::lconfig("pid.z.kP: {}", z_controller.kP);
    logger::lconfig("pid.z.kI: {}", z_controller.kI);
    logger::lconfig("pid.z.kD: {}", z_controller.kD);

    logger::lconfig("pid.r.kP: {}", roll_controller.kP);
    logger::lconfig("pid.r.kI: {}", roll_controller.kI);
    logger::lconfig("pid.r.kD: {}", roll_controller.kD);

    logger::lconfig("pid.p.kP: {}", pitch_controller.kP);
    logger::lconfig("pid.p.kI: {}", pitch_controller.kI);
    logger::lconfig("pid.p.kD: {}", pitch_controller.kD);

    logger::lconfig("pid.vy.kP: {}", vyaw_controller.kP);
    logger::lconfig("pid.vy.kI: {}", vyaw_controller.kI);
    logger::lconfig("pid.vy.kD: {}", vyaw_controller.kD);

    logger::lconfig("pid.trim: {}", trim);
}

void drone::load_configuration(){
    config::load_file();
    
    sensor_ref_rate = config::get_config_int("sensor.ref_rate", 60);
    upper_sensor_freq_cutoff = config::get_config_dbl("sensor.mpu6050.upper_freq_cutoff", 5);
    lower_sensor_freq_cutoff = config::get_config_dbl("sensor.mpu6050.lower_freq_cutoff", 0.01);
    settle_length = config::get_config_int("sensor.settle_length", 200);
    sensor_roll_pitch_tau = config::get_config_dbl("sensor.roll_pitch_tau", 0.02);
    sensor_g_tolerance = config::get_config_dbl("sensor.g_tolerance", 0.1);

    message_thread_ref_rate = config::get_config_int("message.ref_rate", 10);
    socket_path = config::get_config_str("message.socket_path", "./run/drone");

    upper_vz_freq_cutoff = config::get_config_dbl("sensor.vertical_v.upper_freq_cutoff", 2);
    upper_pressure_freq_cutoff = config::get_config_dbl("sensor.pressure.upper_freq_cutoff", 5);
    sensor_z_tau = config::get_config_dbl("sensor.z_tau", 0.02);



    config::write_to_file();

    load_pid_config();
    
    sensor_sleep_int = 1000000 / sensor_ref_rate;
    message_sleep_int = 1000000 / message_thread_ref_rate;
    sensor_g_tolerance_sqrd = sensor_g_tolerance * sensor_g_tolerance;

    logger::lconfig("Settle Length: {}", settle_length);
    logger::lconfig("Sensor Refresh Rate: {}hz", sensor_ref_rate);
    logger::lconfig("Message Refresh Rate: {}hz", message_thread_ref_rate);
    logger::lconfig("Socket Location: {}", socket_path);
    logger::lconfig("Sensor G Tolerance: {}", sensor_g_tolerance);
    logger::lconfig("Sensor Roll Pitch Tau: {}", sensor_roll_pitch_tau);
    logger::lconfig("Sensor Z Tau: {}", sensor_z_tau);
    logger::lconfig("Upper Accelerometer Frequency Cutoff: {}", upper_sensor_freq_cutoff);
    logger::lconfig("Upper Pressure Frequency Cutoff: {}", upper_pressure_freq_cutoff);
    logger::lconfig("Upper Vz Frequency Cutoff: {}", upper_vz_freq_cutoff);

    setup_filters();
}

void drone::enable(bool fl, bool fr, bool bl, bool br){

}

void drone::set_all(double per){
    if(per < 0){
        per = 0;
    }else if(per > 1){
        per = 1;
    }
    motor_bl_spd = motor_br_spd = motor_fl_spd = motor_fr_spd = per;
    #ifdef ENABLE_MOTOR
        int pow = (int)(per * (THROTTLE_MAX - THROTTLE_MIN)) + THROTTLE_MIN;
        pca9685::set_pwm_ms(MOTOR_FL_PIN, pow);
        pca9685::set_pwm_ms(MOTOR_FR_PIN, pow);
        pca9685::set_pwm_ms(MOTOR_BL_PIN, pow);
        pca9685::set_pwm_ms(MOTOR_BR_PIN, pow);
    #endif
}

void drone::set_diagonals(short diagonal, double per){
    if(per < 0){
        per = 0;
    }else if(per > 1){
        per = 1;
    }
    int pow = (int)(per * (THROTTLE_MAX - THROTTLE_MIN)) + THROTTLE_MIN;
    switch (diagonal)
    {
    case FLBR_DIAGONAL:
        motor_br_spd = motor_fl_spd = per;
        #ifdef ENABLE_MOTOR
            pca9685::set_pwm_ms(MOTOR_FL_PIN, pow);
            pca9685::set_pwm_ms(MOTOR_BR_PIN, pow);
        #endif
        break;
    case FRBL_DIAGONAL:
        motor_bl_spd = motor_fr_spd = per;
        #ifdef ENABLE_MOTOR
            pca9685::set_pwm_ms(MOTOR_FR_PIN, pow);
            pca9685::set_pwm_ms(MOTOR_BL_PIN, pow);
        #endif
        break;
    
    default:
        break;
    }
}

void drone::set_motor(short motor, double per){
    if(per < 0){
        per = 0;
    }else if(per > 1){
        per = 1;
    }
    int pow = (int)(per * (THROTTLE_MAX - THROTTLE_MIN)) + THROTTLE_MIN;
    switch(motor){
    case MOTOR_FL:
        motor_fl_spd = per;
        #ifdef ENABLE_MOTOR
            pca9685::set_pwm_ms(MOTOR_FL_PIN, pow);
        #endif
        break;
    case MOTOR_FR:
        motor_fr_spd = per;
        #ifdef ENABLE_MOTOR
            pca9685::set_pwm_ms(MOTOR_FR_PIN, pow);
        #endif
        break;
    case MOTOR_BL:
        motor_bl_spd = per;
        #ifdef ENABLE_MOTOR
            pca9685::set_pwm_ms(MOTOR_BL_PIN, pow);
        #endif
        break;
    case MOTOR_BR:
        motor_br_spd = per;
        #ifdef ENABLE_MOTOR
            pca9685::set_pwm_ms(MOTOR_BR_PIN, pow);
        #endif
        break;
    default:break;
    }
}



void drone::set_throttle(double per){
    throttle = per;
    // drone::set_all(throttle);
}

void drone::destroy(){
    
    set_all(0);

    pca9685::destroy();
}


int next_token(const std::string & tokenized, int i, std::string & out){
    int k = tokenized.find(" ", i);
    if(k < 0){
        out = tokenized.substr(i);
        return tokenized.length();
    }
    out = tokenized.substr(i, k);
    return k + 1;
}


void drone::run_command(const std::string& s){
    std::string k;
    run_command(s, k);
}

void drone::run_command(const std::string& s, std::string& msg){
    std::string command;
    int i = next_token(s, 0, command);
    // string v;
    // i = next_token(m, i, v);
    // float value = atof(v.c_str());
    int len = s.length();

    if(command == "thrtl"){
        i = next_token(s, i, command);
        if(command == ""){
            msg = "Throttle is "+std::to_string(throttle)+"% power.";
        }else {
            float value = atof(command.c_str());
            std::cout << "Throttle Argument: " << command << "\n";
            set_throttle(value);
            msg = "Throttle set to " + std::to_string(value) + "% power.";
        }
    }else if(command == ""){

    }else {
        msg = "I don't know that command! :(";
        return;
    }
}



static std::mutex sensor_thread_mutex, message_thread_mutex;
static std::thread rel_config;

void settle(){
    logger::info("Settling sensors");
    state old = curr_state;
    curr_state = state::settling;
    for(int i = 0; i < settle_length; i++){
        mpu6050::read(mpu6050_data);

        for(int i = 0; i < 6; i ++){
            filtered_mpu6050_data[i] = mpu6050_filters[i][mpu6050_data[i]];
        }

        bmp390_data[0] = bmp390::get_temp();
        bmp390_data[1] = bmp390::get_press(bmp390_data[0]);
        bmp390_data[1] = pressure_filter[bmp390_data[1]];
        bmp390_data[2] = bmp390::get_height(bmp390_data[0], bmp390_data[1]);

        old_altitude = bmp390_data[2];
        initial_altitude = bmp390_data[2];
        usleep(sensor_sleep_int);
    }

    // logger::info("Initial altitude: {:.2f} m", 3.281 * initial_altitude);
    logger::info("Settled sensors");
    curr_state = old;
}

void calibrate(){
    state old = curr_state;
    logger::info("Calibrating sensors");
    logger::info("Calibrating MPU6050");
    curr_state = state::calibrating;
    orientation = math::quarternion(1, 0, 0, 0);

    velocity = math::vector(0, 0, 0);
    position = math::vector(0, 0, 0);

    mpu6050::calibrate(7);

    logger::info("Calibrating BMP390");
    int n = 500;
    for(int i = 0; i < n; i ++){
        bmp390_data[0] = bmp390::get_temp();
        bmp390_data[1] = bmp390::get_press(bmp390_data[0]);
        bmp390_data[1] = pressure_filter[bmp390_data[1]];
        bmp390_data[2] = bmp390::get_height(bmp390_data[0], bmp390_data[1]);

        usleep(sensor_sleep_int);
    }

    double sum = 0;
    for(int i = 0; i < n; i ++){
        bmp390_data[0] = bmp390::get_temp();
        bmp390_data[1] = bmp390::get_press(bmp390_data[0]);
        bmp390_data[1] = pressure_filter[bmp390_data[1]];
        bmp390_data[2] = bmp390::get_height(bmp390_data[0], bmp390_data[1]);
        sum += bmp390_data[2];
        usleep(sensor_sleep_int);
    }

    initial_altitude = old_altitude = sum / n;

    logger::info("Calibrated sensors");
    curr_state = old;
}

void sensor_thread_funct(){
    logger::info("Sensor thread alive!");

    // orientation = math::quarternion(1, 0, 0, 0);

    math::quarternion euler_q;
    math::vector euler_v;
    math::vector temp;
    auto then = std::chrono::steady_clock::now();
    auto start = then;
    auto now = std::chrono::steady_clock::now();

    setup_filters();
    calibrate();
    settle();
    
    curr_state = state::ready;
    while(alive){
        std::lock_guard<std::mutex> sensor_lock_guard(sensor_thread_mutex);
        now = std::chrono::steady_clock::now();
        dt = std::chrono::duration_cast<std::chrono::nanoseconds> (now - then).count() * 0.000000001;
        int t_since = std::chrono::duration_cast<std::chrono::nanoseconds> (now - start).count();
        then = now;
        
        { // MPU6050 Sensor Read & Filter
            mpu6050::read(mpu6050_data);
            mpu6050_data[4] *= -1;
            mpu6050_data[5] *= -1;
            for(int i = 0; i < 6; i ++){
                filtered_mpu6050_data[i] = mpu6050_filters[i][mpu6050_data[i]];
            }
        }

        { // BMP390 Sensor Read & Filter
            old_altitude = bmp390_data[2];
            bmp390_data[0] = bmp390::get_temp();
            bmp390_data[1] = bmp390::get_press(bmp390_data[0]);
            bmp390_data[1] = pressure_filter[bmp390_data[1]];
            bmp390_data[2] = bmp390::get_height(bmp390_data[0], bmp390_data[1]);
            valt = (bmp390_data[2] - old_altitude) / dt;
        }

        {// Dead Reckoning
            euler_v = math::vector(filtered_mpu6050_data[3]*dt*DEG_TO_RAD, filtered_mpu6050_data[4]*dt*DEG_TO_RAD, filtered_mpu6050_data[5]*dt*DEG_TO_RAD);
            euler_q = math::quarternion::fromEulerZYX(euler_v);
            orientation = euler_q*orientation;
            orientation_euler = math::quarternion::toEuler(orientation);


            double a_dist_from_one_sqrd = filtered_mpu6050_data[0] * filtered_mpu6050_data[0] + filtered_mpu6050_data[1] * filtered_mpu6050_data[1] + filtered_mpu6050_data[2] * filtered_mpu6050_data[2] - 1;
            if((a_dist_from_one_sqrd < 0 ? a_dist_from_one_sqrd > - sensor_g_tolerance_sqrd : a_dist_from_one_sqrd < sensor_g_tolerance_sqrd)){
                double roll = atan2(filtered_mpu6050_data[1], filtered_mpu6050_data[2]);
                double pitch = atan2((filtered_mpu6050_data[0]) , sqrt(filtered_mpu6050_data[1] * filtered_mpu6050_data[1] + filtered_mpu6050_data[2] * filtered_mpu6050_data[2]));

                orientation_euler.x = orientation_euler.x * (1 - sensor_roll_pitch_tau) + roll * sensor_roll_pitch_tau;
                orientation_euler.y = orientation_euler.y * (1 - sensor_roll_pitch_tau) + pitch * sensor_roll_pitch_tau;
                
                orientation = math::quarternion::fromEulerZYX(orientation_euler);
                orientation_euler = math::quarternion::toEuler(orientation);
            }

            temp = velocity * dt;
            position = position + temp;
            position.z = position.z * sensor_z_tau + (bmp390_data[2] - initial_altitude) * (1 - sensor_z_tau);
            temp = math::vector(filtered_mpu6050_data[0]*dt*G, filtered_mpu6050_data[1]*dt*G, -filtered_mpu6050_data[2]*dt*G);
            temp = math::quarternion::rotateVector(orientation, temp);
            temp.z += G*dt;
            velocity = velocity + temp;
            // velocity.z = vzfilter[velocity.z];
            velocity.z = velocity.z * sensor_z_tau + valt * (1 - sensor_z_tau);
        }

        {// PID updates
            // z_controller.setpoint = 
            // double z = z_controller.update(position.z, dt);
            double z = 0;
            double r = roll_controller.update(orientation_euler.x, dt);
            double p = pitch_controller.update(orientation_euler.y, dt);
            // double vy = vyaw_controller.update(filtered_mpu6050_data[5], dt);
            double vy = yawthrust;

            // logger::info("p: {:.4f} o: {:.4f}", roll_controller.p, roll_controller.output);

            drone::set_motor(MOTOR_FL, z + r + p + vy + trim + thrust);
            drone::set_motor(MOTOR_FR, z - r + p - vy + trim + thrust);
            drone::set_motor(MOTOR_BL, z + r - p - vy + trim + thrust);
            drone::set_motor(MOTOR_BR, z - r - p + vy + trim + thrust);
        }

        if(zero_flag){
            orientation = math::quarternion(1, 0, 0, 0);

            velocity = math::vector(0, 0, 0);
            position = math::vector(0, 0, 0);
            initial_altitude = old_altitude = bmp390_data[2];
            valt = 0;

            logger::info("Zeroed");
            zero_flag = false;
        }

        if(calib_flag){
            calibrate();
            settle();
            calib_flag = false;
        }
        
        usleep(sensor_sleep_int);
    }
}

void reload_config_thread(){
    std::lock_guard<std::mutex> sensor_lock_guard(sensor_thread_mutex);
    std::lock_guard<std::mutex> message_lock_guard(message_thread_mutex);

    logger::info("Acquired locks!");
    drone::load_configuration();
    logger::info("Releasing locks!");
}


void reconnect_node_server(sock::socket& client, sock::un_connection& unix_connection){
    logger::crit("Lost contact with node server.");
    if(unix_connection.valid){
        unix_connection.close();
    }

    while(alive && !unix_connection.valid) {
        client.close();
        client = sock::socket(sock::unix, sock::tcp);
        unix_connection = client.un_connect(socket_path.c_str());  
        usleep(100000);
    }
    logger::crit("Found node server!");
}


static int substr_chr(char * out, const char * bruh, char c, int a, int b){
    int k = 0;
    for(int i = a; i != b && bruh[i] != '\0'; i ++){
        if(bruh[i] == c){
            out[k++] = '\0';
            return i;
        }
        out[k++] = bruh[i];
    }
    out[k] = '\0';
    return -1;
}



void message_thread_funct(){
    logger::info("Message thread alive!");  

    {
        reporter::bind_dbl("ax", filtered_mpu6050_data); // 0
        reporter::bind_dbl("ay", filtered_mpu6050_data + 1); // 1
        reporter::bind_dbl("az", filtered_mpu6050_data + 2); // 2
        reporter::bind_dbl("vroll", filtered_mpu6050_data + 3); // 3
        reporter::bind_dbl("vpitch", filtered_mpu6050_data + 4); // 4
        reporter::bind_dbl("vyaw", filtered_mpu6050_data + 5); // 5
        reporter::bind_dbl("vx", &velocity.x); // 6
        reporter::bind_dbl("vy", &velocity.y); // 7
        reporter::bind_dbl("vz", &velocity.z); // 8
        reporter::bind_dbl("x", &position.x); // 9
        reporter::bind_dbl("y", &position.y); // 10
        reporter::bind_dbl("z", &position.z); // 11
        reporter::bind_dbl("roll", &orientation_euler.x); // 12
        reporter::bind_dbl("pitch", &orientation_euler.y); // 13
        reporter::bind_dbl("yaw", &orientation_euler.z); // 14
        reporter::bind_dbl("temp", bmp390_data); // 15
        reporter::bind_dbl("pressure", bmp390_data+1); // 16
        reporter::bind_dbl("altitude", bmp390_data+2); // 17
        reporter::bind_dbl("ialt", &initial_altitude); // 18
        reporter::bind_dbl("valt", &valt); // 19
        
        reporter::bind_dbl("zset", &z_controller.setpoint); // 20
        reporter::bind_dbl("vyawset", &vyaw_controller.setpoint); // 21
        reporter::bind_dbl("rollset", &roll_controller.setpoint); // 22
        reporter::bind_dbl("pitchset", &pitch_controller.setpoint); // 23
        reporter::bind_dbl("zerr", &z_controller.err); // 24
        reporter::bind_dbl("vyawerr", &vyaw_controller.err); // 25
        reporter::bind_dbl("rollerr", &roll_controller.err); // 26
        reporter::bind_dbl("pitcherr", &pitch_controller.err); // 27

        reporter::bind_dbl("flpwr", &motor_fl_spd); // 28
        reporter::bind_dbl("frpwr", &motor_fr_spd); // 29
        reporter::bind_dbl("blpwr", &motor_bl_spd); // 30
        reporter::bind_dbl("brpwr", &motor_br_spd); // 31

        reporter::bind_int("state", (int *) (&curr_state)); // 32
        reporter::bind_dbl("cpuusg", &cpu_usg); // 33
        reporter::bind_dbl("battery", &battery); // 34
        reporter::bind_dbl("dt", &dt); // 35
        reporter::bind_bool("controller", &cntrller_connected); // 36

        reporter::bind_dbl("zicurr", &z_controller.i_curr); // 37
        reporter::bind_dbl("vyawicurr", &vyaw_controller.i_curr); // 38
        reporter::bind_dbl("rollicurr", &roll_controller.i_curr); // 39
        reporter::bind_dbl("pitchicurr", &pitch_controller.i_curr); // 40
        reporter::bind_dbl("zderr", &z_controller.derr); // 41
        reporter::bind_dbl("vyawderr", &vyaw_controller.derr); // 42
        reporter::bind_dbl("rollderr", &roll_controller.derr); // 43
        reporter::bind_dbl("pitchderr", &pitch_controller.derr); // 44
        reporter::bind_dbl("zp", &z_controller.p); // 45
        reporter::bind_dbl("vyawp", &vyaw_controller.p); // 46
        reporter::bind_dbl("rollp", &roll_controller.p); // 47
        reporter::bind_dbl("pitchp", &pitch_controller.p); // 48
        reporter::bind_dbl("zi", &z_controller.i); // 49
        reporter::bind_dbl("vyawi", &vyaw_controller.i); // 50
        reporter::bind_dbl("rolli", &roll_controller.i); // 51
        reporter::bind_dbl("pitchi", &pitch_controller.i); // 52
        reporter::bind_dbl("zd", &z_controller.d); // 53
        reporter::bind_dbl("vyawd", &vyaw_controller.d); // 54
        reporter::bind_dbl("rolld", &roll_controller.d); // 55
        reporter::bind_dbl("pitchd", &pitch_controller.d); // 56
        reporter::bind_dbl("zout", &z_controller.output); // 57
        reporter::bind_dbl("vyawout", &vyaw_controller.output); // 58
        reporter::bind_dbl("rollout", &roll_controller.output); // 59
        reporter::bind_dbl("pitchout", &pitch_controller.output); // 60

        reporter::bind_dbl("trim", &trim); // 61
    }
    sock::socket client(sock::unix, sock::tcp);
    sock::un_connection unix_connection = client.un_connect(socket_path.c_str());

    if(!unix_connection.valid){
        reconnect_node_server(client, unix_connection);
    }
    char send[1024];
    char recv[1024];
    char buf[100];

    const char * cock[] = {"zp", "zi", "zd", "vyp", "vyi", "vyd", "rp", "ri", "rd", "pp", "pi", "pd", "trim"};
    while(alive){
        // std::lock_guard<std::mutex> message_lock_guard(message_thread_mutex);
        

        // | zero | calibrate | Reload Config | Chg PID Config |
        // |  0   |     1     |       2       |         3         |
        if(unix_connection.can_read()){
            logger::info("YOO DATA!");
            int len = unix_connection.read(recv, 50);
            logger::info("{}", recv);
            recv[len] = '\0';
            std::string bruv(recv);
            if(len > 0){
                int l = bruv.find(' ');
                int cmd = std::stoi(bruv.substr(0, l));
                // logger::info("");

                state old;
                switch(cmd){
                case 0:
                    logger::info("Zeroing");
                    zero_flag = true;
                    break;
                case 1:
                    logger::info("Calibrating");
                    calib_flag = true;
                    break;
                case 2:
                    old = curr_state;
                    curr_state = state::configuring;
                    logger::info("Reloading configuration");
                    logger::info("Acquired locks!");
                    drone::load_configuration();
                    logger::info("Releasing locks!");
                    // rel_config = std::thread(reload_config_thread);
                    // rel_config.join();
                    curr_state = old;
                    break;
                case 3:
                    {
                        // l = substr_chr(buf, recv, ' ', l, -1);
                        int k = l+1;
                        l = bruv.find(' ', k);
                        if(l != -1){
                            int var = std::stoi(bruv.substr(k,l));
                            double val = std::stof(bruv.substr(l+1));
                            logger::info("Changing {} to {}.", cock[var], val);
                            switch(var){
                            case 0: // z_p
                                z_controller.kP = val;
                                z_controller.reset_integral_term();
                                break;
                            case 1: // z_i
                                z_controller.kI = val;
                                z_controller.reset_integral_term();
                                break;
                            case 2: // z_d
                                z_controller.kD = val;
                                z_controller.reset_integral_term();
                                break;
                            case 3: // vy_p
                                vyaw_controller.kP = val;
                                vyaw_controller.reset_integral_term();
                                break;
                            case 4: // vy_i
                                vyaw_controller.kI = val;
                                vyaw_controller.reset_integral_term();
                                break;
                            case 5: // vy_d
                                vyaw_controller.kD = val;
                                vyaw_controller.reset_integral_term();
                                break;
                            case 6: // r_p
                                roll_controller.kP = val;
                                roll_controller.reset_integral_term();
                                break;
                            case 7: // r_i
                                roll_controller.kI = val;
                                roll_controller.reset_integral_term();
                                break;
                            case 8: // r_d
                                roll_controller.kD = val;
                                roll_controller.reset_integral_term();
                                break;
                            case 9: // p_p
                                pitch_controller.kP = val;
                                pitch_controller.reset_integral_term();
                                break;
                            case 10: // p_i
                                pitch_controller.kI = val;
                                pitch_controller.reset_integral_term();
                                break;
                            case 11: // p_d
                                pitch_controller.kD = val;
                                pitch_controller.reset_integral_term();
                                break;
                            case 12: // trim
                                trim = val;
                                pitch_controller.reset_integral_term();
                            default:
                                logger::warn("Unknown variable \"{}\"", var);
                            }
                        }
                    }
                    break;
                // case 4:
                //     {

                //     }
                //     break;
                default:
                    logger::warn("Unknown cmd \"{}\"", cmd);
                }
            }else{
                reconnect_node_server(client, unix_connection);
            }
            // logger::info("Message: \"{}\"", recv);
        }


        // |                MPU6050                  |                 Dead Reckoned                 |             BMP390                |      BMP390 Related
        // | Ax | Ay | Az | ARroll | ARpitch | ARyaw | Vx | Vy | Vz | X | Y | Z | Roll | Pitch | Yaw | Temperature | Pressure | Altitude | Initial Altitude | Valt |
        // | 0  | 1  | 2  |   3    |    4    |   5   | 6  | 7  | 8  | 9 |10 |11 |  12  |  13   | 14  |     15      |    16    |    17    |         18       |  19  |

        // |        Setpoints        |          Error          |     Motor Speed   |                State and Sysinfo              |
        // | z | vyaw | roll | pitch | z | vyaw | roll | pitch | fl | fr | bl | br | State | CPU Usg % | Battery | dt | controller |
        // |20 |  21  |  22  |  23   |24 |  25  |  26  |  27   | 28 | 29 | 30 | 31 |  32   |    33     |   34    | 35 |     36     |
        
        // |                                                                                            PID Controller Info                                                                                                                                    | trim |
        // |                    i_term                           |                   derr                      |               p                 |                 i               |                 d               |                output                   | trim |
        // | z_i_term | vyaw_i_term | roll_i_term | pitch_i_term | z_derr | vyaw_derr | roll_derr | pitch_derr | z_p | vyaw_p | roll_p | pitch_p | z_i | vyaw_i | roll_i | pitch_i | z_d | vyaw_d | roll_d | pitch_d | z_out | vyaw_out | roll_out | pitch_out | trim |
        // |    37    |      38     |      39     |      40      |   41   |    42     |     43    |      44    | 45  |  46    |   47   |    48   | 49  |   50   |   51   |   52    | 53  |   54   |   55   |   56    |  57   |    58    |    59    |     60    |  61  |

                //     0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61
        sprintf(send, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
            filtered_mpu6050_data[0]*G, filtered_mpu6050_data[1]*G, (filtered_mpu6050_data[2])*G, filtered_mpu6050_data[3]*DEG_TO_RAD, filtered_mpu6050_data[4]*DEG_TO_RAD, filtered_mpu6050_data[5]*DEG_TO_RAD,
            velocity.x, velocity.y, velocity.z, position.x, position.y, position.z, orientation_euler.x, orientation_euler.y, orientation_euler.z,
            bmp390_data[0], bmp390_data[1], bmp390_data[2], initial_altitude, valt,
            z_controller.setpoint, vyaw_controller.setpoint, roll_controller.setpoint, pitch_controller.setpoint,
            z_controller.err, vyaw_controller.err, roll_controller.err, pitch_controller.err,
            motor_fl_spd, motor_fr_spd, motor_bl_spd, motor_br_spd,
            curr_state, -1.0, -1.0, dt, (cntrller_connected ? 1.0 : 0.0),
            z_controller.i_curr, vyaw_controller.i_curr, roll_controller.i_curr, pitch_controller.i_curr,
            z_controller.derr, vyaw_controller.derr, roll_controller.derr, pitch_controller.derr,
            z_controller.p, vyaw_controller.p, roll_controller.p, pitch_controller.p,
            z_controller.i, vyaw_controller.i, roll_controller.i, pitch_controller.i,
            z_controller.d, vyaw_controller.d, roll_controller.d, pitch_controller.d,
            z_controller.output, vyaw_controller.output, roll_controller.output, pitch_controller.output,
            trim
            // debug_vals[0], debug_vals[1], debug_vals[2], debug_vals[3], debug_vals[4], debug_vals[5]);
        );
        // logger::info("state: {}",curr_state);
        int e = unix_connection.send(send, strlen(send));
        if(e < 0) {
            reconnect_node_server(client, unix_connection);
        }



        usleep(message_sleep_int);
    }
}






void drone::init_sensors(bool thread) {
    curr_state = state::init;
    logger::info("Initializing MPU6050.");
    mpu6050::init();
    mpu6050::set_accl_set(mpu6050::accl_range::g_2);
    mpu6050::set_gyro_set(mpu6050::gyro_range::deg_250);
    mpu6050::set_clk(mpu6050::clk::y_gyro);
    mpu6050::set_fsync(mpu6050::fsync::input_dis);
    mpu6050::set_dlpf_bandwidth(mpu6050::dlpf::hz_5);
    mpu6050::wake_up();
    logger::info("Finished intializing the MPU6050.");

    logger::info("Initializing BMP390.");
    bmp390::init();
    bmp390::soft_reset();
    bmp390::set_oversample(bmp390::HIGH, bmp390::ULTRA_LOW_POWER);
    bmp390::set_iir_filter(bmp390::COEFF_3);
    bmp390::set_output_data_rate(bmp390::hz50);
    bmp390::set_enable(true, true);
    bmp390::set_pwr_mode(bmp390::NORMAL);
    logger:info("Finished initializing the BMP390.");

    if(thread){
        logger::info("Starting up sensor thread.");
        sensor_thread = std::thread(sensor_thread_funct);
    }
}

void drone::init_messsage_thread(bool thread){
    if(thread){
        logger::info("Starting up message thread.");
        logger::info("Message rate: {}", message_thread_ref_rate);
        message_thread = std::thread(message_thread_funct);
    }
}



void drone::destroy_message_thread(){
    if(message_thread.joinable()){
        logger::info("Joining message thread.");
        alive = false;
        message_thread.join();
        logger::info("Joined message thread.");
    }
    if(rel_config.joinable()){
        rel_config.join();
    }
}

void drone::destroy_sensors(){
    curr_state = state::destroying;
    if(sensor_thread.joinable()){
        logger::info("Joining sensor thread.");
        alive = false;
        sensor_thread.join();
        logger::info("Joined sensor thread.");
    }
}


void drone::set_trim(double _trim){
    trim = _trim;
}

double drone::get_trim(){
    return trim;
}

double* drone::get_trim_ptr(){
    return &trim;
}
double* drone::get_thrust_ptr(){
    return &thrust;
}
double* drone::get_yawthrust_ptr(){
    return &yawthrust;
}