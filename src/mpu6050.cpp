//-------------------------------MPU6050 Accelerometer and Gyroscope C++ library-----------------------------
//Copyright (c) 2019, Alex Mous
//Licensed under the CC BY-NC SA 4.0

// MODIFIED BY ANDREW LIN

//Include the header file for this class
#include "mpu6050.h"


#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
extern "C" {
	#include <linux/i2c-dev.h>
	#include <i2c/smbus.h>
}
#include <cmath>
#include <thread>
#include <cstdio>
#include <string>
#include <cstdint>

#include <logger.h>

#define Read(r) (uint16_t) i2c_smbus_read_byte_data(fd, r)
#define Write(r,v) i2c_smbus_write_byte_data(fd, r, v)

static int fd;

void mpu6050::init(){init(MPU6050_DEFAULT_ADDR);}

static int offsets[6];

static int accl_scale = 16384;
static double gyro_scale = 16.4;

void mpu6050::init(int addr){
    fd = open("/dev/i2c-1", O_RDWR); //Open the I2C device file
	if (fd < 0) { //Catch errors
		logger::err("Failed to open /dev/i2c-1. Please check that I2C is enabled with raspi-config"); //Print error message
	}

	int status = ioctl(fd, I2C_SLAVE, addr); //Set the I2C bus to use the correct address
	if (status < 0) {
		// std::cout << "ERR (mpu6050.cpp:open()): Could not get I2C bus with " << addr << " address. Please confirm that this address is correct\n"; //Print error message
		logger::err("Could not get I2C bus with {} address. Please confirm that this address is correct", addr);
	}
	
	offsets[0] = X_ACCL_SHIFT;
	offsets[1] = Y_ACCL_SHIFT;
	offsets[2] = Z_ACCL_SHIFT;
	offsets[3] = X_GYRO_SHIFT;
	offsets[4] = Y_GYRO_SHIFT;
	offsets[5] = Z_GYRO_SHIFT;
}


inline void debug2 (const char * name, int reg){
	logger::debug("Register {:20s} : Value of Register {:2x}: {:5d}", name, reg, Read(reg));
}

inline void debug3(int val){
	// printf("[DEBUG] Changing value to %5d", val);
}

void mpu6050::print_debug(){
	debug2("Pwr Mng 1",REG_PWR_MNG_1);
	debug2("Cfg", REG_CFG);
	debug2("Accl Cfg", REG_ACCL_CFG);
	debug2("Gyro Cfg", REG_GYRO_CFG);

}

void mpu6050::wake_up(){
	Write(REG_PWR_MNG_1, Read(REG_PWR_MNG_1) & (~0b01000000));
	usleep(1000);
}

void mpu6050::sleep(){
	Write(REG_PWR_MNG_1, Read(REG_PWR_MNG_1) | 0b01000000);
	usleep(1000);
}

void mpu6050::set_accl_set(accl_range::accl_range set){
	switch(set){
	case accl_range::g_16:
    	accl_scale = 2048;
		break;
	case accl_range::g_8: 
    	accl_scale = 4096;
		break;
	case accl_range::g_4: 
    	accl_scale = 8192;
		break;
	case accl_range::g_2: 
    	accl_scale = 16384;
		break;
	}
	Write(REG_ACCL_CFG, Read(REG_ACCL_CFG) & (~0b00011000) | (set << 3));
	usleep(1000);
}

void mpu6050::set_gyro_set(gyro_range::gyro_range set){
	switch(set){
	case gyro_range::deg_250:
		gyro_scale = 131;
		break;
	case gyro_range::deg_500:
		gyro_scale = 65.5;
		break;
	case gyro_range::deg_1000:
		gyro_scale = 32.8;
		break;
	case gyro_range::deg_2000:
		gyro_scale = 16.4;
		break;
	}
	Write(REG_GYRO_CFG, Read(REG_GYRO_CFG) & (~0b00011000) | (set << 3));
	usleep(1000);


}

void mpu6050::set_clk(clk::clk set){
	Write(REG_PWR_MNG_1, Read(REG_PWR_MNG_1) & (~0b00000111) | set);
	usleep(1000);
}

void mpu6050::set_dlpf_bandwidth(dlpf::dlpf set){
	Write(REG_CFG, Read(REG_CFG) & (~0b00000111) | set);
	
	usleep(1000);
}

void mpu6050::set_fsync(fsync::fsync set){
	Write(REG_CFG, Read(REG_CFG) & (~0b00111000) | (set << 3));
	usleep(1000);
}

void mpu6050::set_pwr_set(int set){

}

int16_t handle_neg(int n){
	// if(n & 0x4000){
	// 	return -((~(n - 1))&0x7FFF);
	// }
	// return n&0x7FFF;
	return (int16_t) n;
}

void mpu6050::read_raw(int * data){
	data[0] = handle_neg(Read(OUT_XACCL_H) << 8 | Read(OUT_XACCL_L));
	data[1] = handle_neg(Read(OUT_YACCL_H) << 8 | Read(OUT_YACCL_L));
	data[2] = handle_neg(Read(OUT_ZACCL_H) << 8 | Read(OUT_ZACCL_L));
	data[3] = handle_neg(Read(OUT_XGYRO_H) << 8 | Read(OUT_XGYRO_L));
	data[4] = handle_neg(Read(OUT_YGYRO_H) << 8 | Read(OUT_YGYRO_L));
	data[5] = handle_neg(Read(OUT_ZGYRO_H) << 8 | Read(OUT_ZGYRO_L));
}

void mpu6050::read_wo_offsets(double * data){
	data[0] = ((double) (handle_neg(Read(OUT_XACCL_H) << 8 | Read(OUT_XACCL_L)))) / accl_scale;
	data[1] = ((double) (handle_neg(Read(OUT_YACCL_H) << 8 | Read(OUT_YACCL_L)))) / accl_scale;
	data[2] = ((double) (handle_neg(Read(OUT_ZACCL_H) << 8 | Read(OUT_ZACCL_L)))) / accl_scale;
	data[3] = ((double) (handle_neg(Read(OUT_XGYRO_H) << 8 | Read(OUT_XGYRO_L)))) / gyro_scale;
	data[4] = ((double) (handle_neg(Read(OUT_YGYRO_H) << 8 | Read(OUT_YGYRO_L)))) / gyro_scale;
	data[5] = ((double) (handle_neg(Read(OUT_ZGYRO_H) << 8 | Read(OUT_ZGYRO_L)))) / gyro_scale;
}


void mpu6050::read(double * data){
	data[0] = ((double) (handle_neg(Read(OUT_XACCL_H) << 8 | Read(OUT_XACCL_L))) - offsets[0]) / accl_scale;
	data[1] = ((double) (handle_neg(Read(OUT_YACCL_H) << 8 | Read(OUT_YACCL_L))) - offsets[1]) / accl_scale;
	data[2] = ((double) (handle_neg(Read(OUT_ZACCL_H) << 8 | Read(OUT_ZACCL_L))) - offsets[2]) / accl_scale;
	data[3] = ((double) (handle_neg(Read(OUT_XGYRO_H) << 8 | Read(OUT_XGYRO_L))) - offsets[3]) / gyro_scale;
	data[4] = ((double) (handle_neg(Read(OUT_YGYRO_H) << 8 | Read(OUT_YGYRO_L))) - offsets[4]) / gyro_scale;
	data[5] = ((double) (handle_neg(Read(OUT_ZGYRO_H) << 8 | Read(OUT_ZGYRO_L))) - offsets[5]) / gyro_scale;
}

int mpu6050::query_register(int reg){
	return Read(reg);
}

void mpu6050::set_register(int reg, int data){
	Write(reg,data);
}


void mpu6050::calibrate(int n){
	int data[6];
	double error_sum[6];
	double kP[6] = {0.3, 0.3, 0.3, 0.3, 0.3, 0.3};
	double kI[6] = {90, 90, 90, 20, 20, 20};
	double expect[6] = {0, 0, accl_scale * 1.0, 0, 0, 0};
	for(int i = 0; i < 6; i++){
		offsets[i] = 0;
		error_sum[i] = 0;
	}

	for(int i = 0; i < n; i ++){
		for(int j = 0; j < 100; j ++){
			mpu6050::read_raw(data);

			double dt = 0.001;
			logger::debug("{:6d} | {:6d} | {:6d} | {:6d} | {:6d} | {:6d}",data[0],data[1],data[2],data[3],data[4],data[5]);
			for(int k = 0; k < 6; k ++){
				double error = expect[k] - (data[k] - offsets[k]);
				double p_term = error * kP[k];
				error_sum[k] += dt * error * kI[k];

				offsets[k] -= round((p_term + error_sum[k]) / 4);
			}

			usleep(1000);
		}
		

		for(int j = 0; j < 6; j++){
			kP[j] *= 0.75;
			kI[j] *= 0.75;
			error_sum[j] = 0;
		}
	}

	logger::info("Offset: {:6d} | {:6d} | {:6d} | {:6d} | {:6d} | {:6d}",offsets[0], offsets[1], offsets[2], offsets[3], offsets[4], offsets[5]);

}

void mpu6050::set_offsets(int x_a, int y_a, int z_a, int x_g, int y_g, int z_g){
	offsets[0] = x_a;
	offsets[1] = y_a;
	offsets[2] = z_a;
	offsets[3] = x_g;
	offsets[4] = y_g;
	offsets[5] = z_g;
}



void mpu6050::mpu6050::mpu6050(){
    fd = open("/dev/i2c-1", O_RDWR); //Open the I2C device file
	if (fd < 0) { //Catch errors
		logger::err("Failed to open /dev/i2c-1. Please check that I2C is enabled with raspi-config"); //Print error message
	}

	int status = ioctl(fd, I2C_SLAVE, MPU6050_DEFAULT_ADDR); //Set the I2C bus to use the correct address
	if (status < 0) {
		// std::cout << "ERR (mpu6050.cpp:open()): Could not get I2C bus with " << addr << " address. Please confirm that this address is correct\n"; //Print error message
		logger::err("Could not get I2C bus with {} address. Please confirm that this address is correct", MPU6050_DEFAULT_ADDR);
	}
	
	offsets[0] = X_ACCL_SHIFT;
	offsets[1] = Y_ACCL_SHIFT;
	offsets[2] = Z_ACCL_SHIFT;
	offsets[3] = X_GYRO_SHIFT;
	offsets[4] = Y_GYRO_SHIFT;
	offsets[5] = Z_GYRO_SHIFT;

}

void mpu6050::mpu6050::mpu6050(int addr){
    fd = open("/dev/i2c-1", O_RDWR); //Open the I2C device file
	if (fd < 0) { //Catch errors
		logger::err("Failed to open /dev/i2c-1. Please check that I2C is enabled with raspi-config"); //Print error message
	}

	int status = ioctl(fd, I2C_SLAVE, addr); //Set the I2C bus to use the correct address
	if (status < 0) {
		// std::cout << "ERR (mpu6050.cpp:open()): Could not get I2C bus with " << addr << " address. Please confirm that this address is correct\n"; //Print error message
		logger::err("Could not get I2C bus with {} address. Please confirm that this address is correct", addr);
	}
	
	offsets[0] = X_ACCL_SHIFT;
	offsets[1] = Y_ACCL_SHIFT;
	offsets[2] = Z_ACCL_SHIFT;
	offsets[3] = X_GYRO_SHIFT;
	offsets[4] = Y_GYRO_SHIFT;
	offsets[5] = Z_GYRO_SHIFT;
}

void mpu6050::mpu6050::print_debug(){
	debug2("Pwr Mng 1",REG_PWR_MNG_1);
	debug2("Cfg", REG_CFG);
	debug2("Accl Cfg", REG_ACCL_CFG);
	debug2("Gyro Cfg", REG_GYRO_CFG);

}

void mpu6050::mpu6050::wake_up(){
	Write(REG_PWR_MNG_1, Read(REG_PWR_MNG_1) & (~0b01000000));
	usleep(1000);
}

void mpu6050::mpu6050::sleep(){
	Write(REG_PWR_MNG_1, Read(REG_PWR_MNG_1) | 0b01000000);
	usleep(1000);
}

void mpu6050::mpu6050::set_accl_set(accl_range::accl_range set){
	switch(set){
	case accl_range::g_16:
    	accl_scale = 2048;
		break;
	case accl_range::g_8: 
    	accl_scale = 4096;
		break;
	case accl_range::g_4: 
    	accl_scale = 8192;
		break;
	case accl_range::g_2: 
    	accl_scale = 16384;
		break;
	}
	Write(REG_ACCL_CFG, Read(REG_ACCL_CFG) & (~0b00011000) | (set << 3));
	usleep(1000);
}

void mpu6050::mpu6050::set_gyro_set(gyro_range::gyro_range set){
	switch(set){
	case gyro_range::deg_250:
		gyro_scale = 131;
		break;
	case gyro_range::deg_500:
		gyro_scale = 65.5;
		break;
	case gyro_range::deg_1000:
		gyro_scale = 32.8;
		break;
	case gyro_range::deg_2000:
		gyro_scale = 16.4;
		break;
	}
	Write(REG_GYRO_CFG, Read(REG_GYRO_CFG) & (~0b00011000) | (set << 3));
	usleep(1000);
}

void mpu6050::mpu6050::set_clk(clk::clk set){
	Write(REG_PWR_MNG_1, Read(REG_PWR_MNG_1) & (~0b00000111) | set);
	usleep(1000);
}

void mpu6050::mpu6050::set_dlpf_bandwidth(dlpf::dlpf set){
	Write(REG_CFG, Read(REG_CFG) & (~0b00000111) | set);
	
	usleep(1000);
}

void mpu6050::mpu6050::set_fsync(fsync::fsync set){
	Write(REG_CFG, Read(REG_CFG) & (~0b00111000) | (set << 3));
	usleep(1000);
}

void mpu6050::mpu6050::set_pwr_set(int set){

}

void mpu6050::mpu6050::read_raw(){
	raw_data[0] = handle_neg(Read(OUT_XACCL_H) << 8 | Read(OUT_XACCL_L));
	raw_data[1] = handle_neg(Read(OUT_YACCL_H) << 8 | Read(OUT_YACCL_L));
	raw_data[2] = handle_neg(Read(OUT_ZACCL_H) << 8 | Read(OUT_ZACCL_L));
	raw_data[3] = handle_neg(Read(OUT_XGYRO_H) << 8 | Read(OUT_XGYRO_L));
	raw_data[4] = handle_neg(Read(OUT_YGYRO_H) << 8 | Read(OUT_YGYRO_L));
	raw_data[5] = handle_neg(Read(OUT_ZGYRO_H) << 8 | Read(OUT_ZGYRO_L));
}

void mpu6050::mpu6050::read(){
	data[0] = ((double) (handle_neg(Read(OUT_XACCL_H) << 8 | Read(OUT_XACCL_L))) - offsets[0]) / accl_scale;
	data[1] = ((double) (handle_neg(Read(OUT_YACCL_H) << 8 | Read(OUT_YACCL_L))) - offsets[1]) / accl_scale;
	data[2] = ((double) (handle_neg(Read(OUT_ZACCL_H) << 8 | Read(OUT_ZACCL_L))) - offsets[2]) / accl_scale;
	data[3] = ((double) (handle_neg(Read(OUT_XGYRO_H) << 8 | Read(OUT_XGYRO_L))) - offsets[3]) / gyro_scale;
	data[4] = ((double) (handle_neg(Read(OUT_YGYRO_H) << 8 | Read(OUT_YGYRO_L))) - offsets[4]) / gyro_scale;
	data[5] = ((double) (handle_neg(Read(OUT_ZGYRO_H) << 8 | Read(OUT_ZGYRO_L))) - offsets[5]) / gyro_scale;
}

int mpu6050::mpu6050::query_register(int reg){
	return Read(reg);
}

void mpu6050::mpu6050::set_register(int reg, int data){
	Write(reg,data);
}


void mpu6050::mpu6050::calibrate(int n){
	int data[6];
	double error_sum[6];
	double kP[6] = {0.3, 0.3, 0.3, 0.3, 0.3, 0.3};
	double kI[6] = {90, 90, 90, 20, 20, 20};
	double expect[6] = {0, 0, accl_scale * 1.0, 0, 0, 0};
	for(int i = 0; i < 6; i++){
		offsets[i] = 0;
		error_sum[i] = 0;
	}

	for(int i = 0; i < n; i ++){
		for(int j = 0; j < 100; j ++){
			mpu6050::read_raw(data);

			double dt = 0.001;
			logger::debug("{:6d} | {:6d} | {:6d} | {:6d} | {:6d} | {:6d}",data[0],data[1],data[2],data[3],data[4],data[5]);
			for(int k = 0; k < 6; k ++){
				double error = expect[k] - (data[k] - offsets[k]);
				double p_term = error * kP[k];
				error_sum[k] += dt * error * kI[k];

				offsets[k] -= round((p_term + error_sum[k]) / 4);
			}

			usleep(1000);
		}
		

		for(int j = 0; j < 6; j++){
			kP[j] *= 0.75;
			kI[j] *= 0.75;
			error_sum[j] = 0;
		}
	}

	logger::info("Offset: {:6d} | {:6d} | {:6d} | {:6d} | {:6d} | {:6d}",offsets[0], offsets[1], offsets[2], offsets[3], offsets[4], offsets[5]);

}

