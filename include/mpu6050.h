//-------------------------------MPU6050 Accelerometer and Gyroscope C++ library-----------------------------
//Copyright (c) 2019, Alex Mous
//Licensed under the CC BY-NC SA 4.0

// MODIFIED BY ANDREW LIN

//-----------------------MODIFY THESE PARAMETERS-----------------------

#define GYRO_RANGE 0 //Select which gyroscope range to use (see the table below) - Default is 0
//	Gyroscope Range
//	0	+/- 250 degrees/second
//	1	+/- 500 degrees/second
//	2	+/- 1000 degrees/second
//	3	+/- 2000 degrees/second
//See the MPU6000 Register Map for more information


#define ACCEL_RANGE 0 //Select which accelerometer range to use (see the table below) - Default is 0
//	Accelerometer Range
//	0	+/- 2g
//	1	+/- 4g
//	2	+/- 8g
//	3	+/- 16g
//See the MPU6000 Register Map for more information

//-----------------------END MODIFY THESE PARAMETERS-----------------------

#define MPU6050_DEFAULT_ADDR 0x68

#define _POSIX_C_SOURCE 200809L //Used for calculating time
#define RAD_T_DEG 57.29577951308 //Radians to degrees (180/PI)

//Select the appropriate settings
#if GYRO_RANGE == 1
	#define GYRO_SENS 65.5
	#define GYRO_CONFIG 0b00001000
#elif GYRO_RANGE == 2
	#define GYRO_SENS 32.8
	#define GYRO_CONFIG 0b00010000
#elif GYRO_RANGE == 3
	#define GYRO_SENS 16.4
	#define GYRO_CONFIG 0b00011000
#else //Otherwise, default to 0
	#define GYRO_SENS 131.0
	#define GYRO_CONFIG 0b00000000
#endif
#undef GYRO_RANGE


#if ACCEL_RANGE == 1
	#define ACCEL_SENS 8192.0
	#define ACCEL_CONFIG 0b00001000
#elif ACCEL_RANGE == 2
	#define ACCEL_SENS 4096.0
	#define ACCEL_CONFIG 0b00010000
#elif ACCEL_RANGE == 3
	#define ACCEL_SENS 2048.0
	#define ACCEL_CONFIG 0b00011000
#else //Otherwise, default to 0
	#define ACCEL_SENS 16384.0
	#define ACCEL_CONFIG 0b00000000
#endif
#undef ACCEL_RANGE

#define REG_PWR_MNG_1 0x6B // Register map 4.28
#define REG_PWR_MNG_2 0x6C // Register map 4.29
#define REG_CFG 0x1A
#define REG_GYRO_CFG 0x1B
#define REG_ACCL_CFG 0x1C
#define REG_FIFO_EN 0x23
#define REG_ACCL_OUT_STRT 0x3B
#define REG_TEMP_OUT_STRT 0x41
#define REG_GYRO_OUT_STRT 0x43
#define REG_SIG_PTH_RST 0x68 //Signal Path Reset
#define REG_USR_CTRL 0x6A

#define OUT_XACCL_H 0x3B
#define OUT_XACCL_L 0x3C
#define OUT_YACCL_H 0x3D
#define OUT_YACCL_L 0x3E
#define OUT_ZACCL_H 0x3F
#define OUT_ZACCL_L 0x40
#define OUT_TEMP_H 0x41
#define OUT_TEMP_L 0x42
#define OUT_XGYRO_H 0x43
#define OUT_XGYRO_L 0x44
#define OUT_YGYRO_H 0x45
#define OUT_YGYRO_L 0x46
#define OUT_ZGYRO_H 0x47
#define OUT_ZGYRO_L 0x48

#define X_ACCL_SHIFT 1265
#define Y_ACCL_SHIFT 215
#define Z_ACCL_SHIFT 1066
#define X_GYRO_SHIFT 65437
#define Y_GYRO_SHIFT 72
#define Z_GYRO_SHIFT 2337


namespace mpu6050 {
	namespace accl_range{
	enum accl_range {
		g_2 = 0b00,
		g_4 = 0b01,
		g_8 = 0b10,
		g_16 = 0b11,
	};
	};
namespace gyro_range {
	enum gyro_range {
		deg_250,
		deg_500,
		deg_1000,
		deg_2000,
	};
	};
	namespace fsync {
	enum fsync {
		input_dis,
		temp_out_l,
		gyro_x_out_l,
		gyro_y_out_l,
		gyro_z_out_l,
		accl_x_out_l,
		accl_y_out_l,
		accl_z_out_l,
	};
	};
	namespace dlpf{
	enum dlpf {
		hz_260,
		hz_184,
		hz_94,
		hz_44,
		hz_21,
		hz_10,
		hz_5,
	};
	};
	namespace clk{
		enum clk {
			int_oscl,
			x_gyro,
			y_gyro,
			z_gyro,
			ext_32kHz,
			ext_19MHz,
			reserved,
			stop
		};
	};


	void init();
	void init(int addr);

	void wake_up();
	void sleep();

	void set_pwr_set(int set);
	void set_accl_set(accl_range::accl_range set);
	void set_gyro_set(gyro_range::gyro_range set);
	void set_clk(clk::clk set);
	void set_dlpf_bandwidth(dlpf::dlpf set);
	void set_fsync(fsync::fsync set);
	
	void set_offsets(int x_a, int y_a, int z_a, int x_g, int y_g, int z_g);

	void read_raw(int * data);
	void read_accl_raw(int * data);
	void read_gyro_raw(int * data);

	void read(double * data);
	void read_accl(double * data);
	void read_gyro(double * data);
	
	void read_wo_offsets(double * data);

	int query_register(int reg);
	void set_register(int reg, int data);

	void calibrate(int n);

	void print_debug();

	void destroy();


	struct mpu6050 {
		int fd;
		int raw_data[6];
		double data[6];

		mpu6050();
		mpu6050(int addr);

		void wake_up();
		void sleep();

		void set_pwr_set(int set);
		void set_accl_set(accl_range::accl_range set);
		void set_gyro_set(gyro_range::gyro_range set);
		void set_clk(clk::clk set);
		void set_dlpf_bandwidth(dlpf::dlpf set);
		void set_fsync(fsync::fsync set);

		void read_raw();

		void read();
		
		int query_register(int reg);
		void set_register(int reg, int data);

		void calibrate(int n);

		void print_debug();
		void destroy();
	};

};
