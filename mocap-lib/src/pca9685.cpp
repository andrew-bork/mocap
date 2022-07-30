#include <pca9685.h>

extern "C" {
	#include <linux/i2c-dev.h>
	#include <i2c/smbus.h>
}

#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
// #include <cstdio>

// #include <INIReader.h>
#include <config.h>
#include <logger.h>

#define read(r) i2c_smbus_read_byte_data(fd, r)
#define write(r,v) i2c_smbus_write_byte_data(fd, r, v)

static int fd = -1;

int pwm_regs[16][4] = {
    {0x06, 0x07, 0x08, 0x09},
    {0x0A, 0x0B, 0x0C, 0x0D},
    {0x0E, 0x0F, 0x10, 0x11},
    {0x02, 0x03, 0x14, 0x15},
    {0x16, 0x17, 0x18, 0x19},
    {0x1A, 0x1B, 0x1C, 0x1D},
    {0x1E, 0x1F, 0x20, 0x21},
    {0x22, 0x23, 0x24, 0x25},
    {0x26, 0x27, 0x28, 0x29},
    {0x2A, 0x2B, 0x2C, 0x2D},
    {0x2E, 0x2F, 0x30, 0x31},
    {0x32, 0x33, 0x34, 0x35},
    {0x36, 0x37, 0x38, 0x39},
    {0x3A, 0x3B, 0x3C, 0x3D},
    {0x3E, 0x3F, 0x40, 0x41},
    {0x42, 0x43, 0x44, 0x45},
};


static int freq = 200; // hz
static int per =  1000000 / freq; // 5 ms

static int _get_frequency(){
    return 25000000 / ((read(PRESCALE) + 1) * 4096);
}

void pca9685::init(){
    logger::info("Initializing pca9685");

    // INIReader reader

    fd = open("/dev/i2c-1", O_RDWR); //Open the I2C device file
	if (fd < 0) { //Catch errors
        logger::err("Failed to open \"/dev/i2c-1\". Please check that I2C is enabled with raspi-config.");
		// std::cout << "ERR (pca9685.cpp:open()): Failed to open /dev/i2c-1. Please check that I2C is enabled with raspi-config\n"; //Print error message
	}

	int status = ioctl(fd, I2C_SLAVE, PCA9685_ADDRESS); //Set the I2C bus to use the correct address
	if (status < 0) {
        logger::err("Could not get i2c bus device with address \"{}\". Please confirm that this address is correct.", PCA9685_ADDRESS);
		// std::cout << "ERR (pca9685.cpp:open()): Could not get I2C bus with " << PCA9685_ADDRESS << " address. Please confirm that this address is correct\n"; //Print error message
	}

    freq = _get_frequency();
    per = 1000000 / freq;
}

void pca9685::destroy(){
    close(fd);
}

int pca9685::query_reg(int reg){
    return i2c_smbus_read_byte_data(fd, reg);
}

void pca9685::write_reg(int reg, int val){
    i2c_smbus_write_byte_data(fd, reg, val);
}

inline int pcaround(double d){
    if(d < 0){
        return (((-d) - ((int)-d)) < 0.5 ? ((int) d) : ((int) d - 1));
    }
    return (d - ((int) d) < 0.5 ? (int) d : (int) d + 1);
}

void pca9685::set_frequency(int frq){
// PRESCALE_VAL -> round(25000000 / (4096 * update_rate)) - 1
    freq = frq;
    per = 1000000 / frq;
    int reg_val = pcaround(25000000.0 / (4096 * freq)) - 1;

    // printf("\n\npca9685::set_frequency\nSetting frequency to %d\nSetting PRESCALE to %d\n", frq, reg_val & 0xFF);
    write(PRESCALE, reg_val & 0xFF);
    usleep(5000);
    // printf("Reg 0x%#d val: %d\n", PRESCALE, read(PRESCALE));
}
void pca9685::set_pwm_ms(int pwm, int micro_s){
    int cycle = 4096 * micro_s / per;
    // printf("\n\npca9685::set_pwm_ms\nSetting OFF of pin %d to %d (%d %d)\nSetting ON of pin %d to %d (%d %d)\n", pwm, cycle, (cycle & 0xF00) >> 8, cycle & 0xFF, pwm, 0, 0, 0);
    write(pwm_regs[pwm][0], 0);
    write(pwm_regs[pwm][1], 0);
    write(pwm_regs[pwm][2], cycle & 0xFF);
    write(pwm_regs[pwm][3], (cycle >> 8) & 0xF);
    // printf("Reg 0x%#d val: %d\nReg 0x%#d val: %d\nReg 0x%#d val: %d\nReg 0x%#d val: %d\n", pwm_regs[pwm][0], read(pwm_regs[pwm][0]), pwm_regs[pwm][1], read(pwm_regs[pwm][1]), pwm_regs[pwm][2], read(pwm_regs[pwm][2]), pwm_regs[pwm][3], read(pwm_regs[pwm][3]));
}
void pca9685::set_pwm_percent(int pwm, double percent){
    int cycle = (int) (4096 * percent);
    // printf("\n\npca9685::set_pwm_ms\nSetting OFF of pin %d to %d (%d %d)\nSetting ON of pin %d to %d (%d %d)\n", pwm, cycle, (cycle & 0xF00) >> 8, cycle & 0xFF, pwm, 0, 0, 0);
    // printf("Estimated OFF ms: %d\nEstimated ON ms: %d\n", percent * per, 0);
    write(pwm_regs[pwm][0], 0);
    write(pwm_regs[pwm][1], 0);
    write(pwm_regs[pwm][2], cycle & 0xFF);
    write(pwm_regs[pwm][3], (cycle >> 8) & 0xF);
    // printf("Reg 0x%#d val: %d\nReg 0x%#d val: %d\nReg 0x%#d val: %d\nReg 0x%#d val: %d\n", pwm_regs[pwm][0], read(pwm_regs[pwm][0]), pwm_regs[pwm][1], read(pwm_regs[pwm][1]), pwm_regs[pwm][2], read(pwm_regs[pwm][2]), pwm_regs[pwm][3], read(pwm_regs[pwm][3]));
}
void pca9685::set_pwm_on(int pwm, int on){
    write(pwm_regs[pwm][0], on & 0xFF);
    write(pwm_regs[pwm][1], (on >> 8) & 0xF);
}
void pca9685::set_pwm_off(int pwm, int off){
    
}

void pca9685::wake_up(){
    int old = read(MODE_1);
    // printf("\n\npca9685::wake_up\nSetting MODE_1 to %d\n", old & ~(0b00010000));
    write(MODE_1, old & ~(0b00010000));
    usleep(5000);
    // printf("Reg 0x%#d val: %d\n", MODE_1, read(MODE_1));
}

void pca9685::sleep(){
    int old = read(MODE_1) & 0xFF;
    // printf("\n\npca9685::sleep\nSetting MODE_1 to %d\n", old | 0b00010000);
    write(MODE_1, old | 0b00010000);
    usleep(5000);
    // printf("Reg 0x%#d val: %d\n", MODE_1, read(MODE_1));
}

void pca9685::restart(){
    
}