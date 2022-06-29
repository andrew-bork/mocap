#include <mpu6050.h>
#include <unistd.h>
#include <iostream>

#include <socket.h>
#include <logger.h>

int main(){
    logger::info("Initializing mpu6050.");
    mpu6050::init();
    mpu6050::set_accl_set(mpu6050::accl_range::g_16);
    mpu6050::set_gyro_set(mpu6050::gyro_range::deg_1000);
    mpu6050::set_clk(mpu6050::clk::x_gyro);
    mpu6050::set_fsync(mpu6050::fsync::input_dis);
    mpu6050::set_dlpf_bandwidth(mpu6050::dlpf::hz_184);
    mpu6050::wake_up();
    logger::info("Finished initializing mpu6050.");
    // setvbuf(stdout, NULL,_IONBF,0);
    

    
    float x,y,z;
    double data[6];

    logger::info("Setting up sensor loop.");
    while(1){
        mpu6050::read(data);
        logger::debug("in loop");
        // std::cout<< data[0] << " " << data[1] << " " << data[2] << " " << data[3] << " " << data[4] << " " << data[5] << "\n";
        usleep(1000);
    }

    
}