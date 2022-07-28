#include <mpu6050.h>
#include <reporter.h>
#include <socket.h>
#include <unistd.h>
#include <logger.h>
#include <math.h>
#include <string>
#include <filter.h>
#include <chrono>
#include <cmath>
#include <config.h>
#include <thread>

#include <cstdlib>

enum state {
    configuring = 0, ready = 1, calibrating = 2, idle = 3, init = 4, settling = 5, destroying = 6
};

math::quarternion orientation;
math::vector orientation_euler;
math::vector position, velocity;

double mpu6050_data[6];
double filtered_mpu6050_data[6];
double mpu6050_filters[6];

double dt;

double upper_sensor_freq_cutoff;
double sensor_g_tolerance;
double sensor_g_tolerance_sqrd;
double sensor_roll_pitch_tau;

int settle_length = 200;
int sensor_sleep_int, message_sleep_int;
int sensor_ref_rate, message_thread_ref_rate;

std::string socket_path;

bool alive = true, zero_flag = false, calib_flag = false;
state curr_state = state::ready;

void setup_filters(){
    for(int i = 0; i < 3; i ++){
        mpu6050_filters[i] = filter::low_pass(sensor_ref_rate, upper_sensor_freq_cutoff);
    }
    for(int i = 3; i < 6; i ++){
        mpu6050_filters[i] = filter::none();
    }

    mpu6050_filters[5] = filter::low_pass(sensor_ref_rate, upper_sensor_freq_cutoff);
}


void config(){
    config::load_file();
    
    sensor_ref_rate = config::get_config_int("sensor.ref_rate", 60);
    upper_sensor_freq_cutoff = config::get_config_dbl("sensor.mpu6050.upper_freq_cutoff", 5);
    // lower_sensor_freq_cutoff = config::get_config_dbl("sensor.mpu6050.lower_freq_cutoff", 0.01);
    settle_length = config::get_config_int("sensor.settle_length", 200);
    sensor_roll_pitch_tau = config::get_config_dbl("sensor.roll_pitch_tau", 0.02);
    sensor_g_tolerance = config::get_config_dbl("sensor.g_tolerance", 0.1);

    message_thread_ref_rate = config::get_config_int("message.ref_rate", 10);
    socket_path = config::get_config_str("message.socket_path", "./run/msg.socket");

    // upper_vz_freq_cutoff = config::get_config_dbl("sensor.vertical_v.upper_freq_cutoff", 2);
    // upper_pressure_freq_cutoff = config::get_config_dbl("sensor.pressure.upper_freq_cutoff", 5);
    // sensor_z_tau = config::get_config_dbl("sensor.z_tau", 0.02);

    config::write_to_file();

    // load_pid_config();
    
    sensor_sleep_int = 1000000 / sensor_ref_rate;
    message_sleep_int = 1000000 / message_thread_ref_rate;
    sensor_g_tolerance_sqrd = sensor_g_tolerance * sensor_g_tolerance;

    logger::lconfig("Settle Length: {}", settle_length);
    logger::lconfig("Sensor Refresh Rate: {}hz", sensor_ref_rate);
    logger::lconfig("Message Refresh Rate: {}hz", message_thread_ref_rate);
    logger::lconfig("Socket Location: {}", socket_path);
    logger::lconfig("Sensor G Tolerance: {}", sensor_g_tolerance);
    logger::lconfig("Sensor Roll Pitch Tau: {}", sensor_roll_pitch_tau);
    // logger::lconfig("Sensor Z Tau: {}", sensor_z_tau);
    logger::lconfig("Upper Accelerometer Frequency Cutoff: {}", upper_sensor_freq_cutoff);
    // logger::lconfig("Upper Pressure Frequency Cutoff: {}", upper_pressure_freq_cutoff);
    // logger::lconfig("Upper Vz Frequency Cutoff: {}", upper_vz_freq_cutoff);

    setup_filters();

}

void settle(){
    state old = curr_state;
    curr_state = state::settling;
    logger::info("Settling sensors");
    for(int i = 0; i < settle_length; i++){
        mpu6050::read(mpu6050_data);
        for(int i = 0; i < 6; i ++){
            filtered_mpu6050_data[i] = mpu6050_filters[i][mpu6050_data[i]];
        }
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
        // std::lock_guard<std::mutex> sensor_lock_guard(sensor_thread_mutex);
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

        {// Dead Reckoning
            {// Rotation
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
            }
        }

        if(zero_flag){
            orientation = math::quarternion(1, 0, 0, 0);
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
    if(unix_connection.valid){
        logger::warn("Found node server!");
    }
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
        // reporter::bind_dbl("vx", &velocity.x); // 6
        // reporter::bind_dbl("vy", &velocity.y); // 7
        // reporter::bind_dbl("vz", &velocity.z); // 8
        // reporter::bind_dbl("x", &position.x); // 9
        // reporter::bind_dbl("y", &position.y); // 10
        // reporter::bind_dbl("z", &position.z); // 11
        reporter::bind_dbl("roll", &orientation_euler.x); // 12
        reporter::bind_dbl("pitch", &orientation_euler.y); // 13
        reporter::bind_dbl("yaw", &orientation_euler.z); // 14
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
                default:
                    logger::warn("Unknown cmd \"{}\"", cmd);
                }
            }else{
                reconnect_node_server(client, unix_connection);
            }
            // logger::info("Message: \"{}\"", recv);
        }


        std::string report = reporter::get_json_report();
        if(unix_connection.send(report.c_str(), report.length()) < 0){
            reconnect_node_server(client, unix_connection);
        }

        usleep(message_sleep_int);
    }
}


void exit_handle(){
    alive = false;
}

int main(){
    std::atexit(exit_handle);
    config();
    std::thread sensor_thread = std::thread(sensor_thread_funct);
    std::thread message_thread = std::thread(message_thread_funct);

    while(alive){usleep(10000);}

    sensor_thread.join();
    message_thread.join();
    
    return 0;
}