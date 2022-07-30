#ifndef PCA9685_H
#define PCA9685_H


#define PCA9685_ADDRESS 0x40
#define PCA9685_ADDRESS_B 0x70

#define PCA9685_SLEEP 0b00010000

// REGISTERS

#define MODE_1      0x00
#define MODE_2      0x01
#define PRESCALE    0xFE

#define ALL_PWM_ON_L    0xFA
#define ALL_PWM_ON_H    0xFB
#define ALL_PWM_OFF_L   0xFC
#define ALL_PWM_OFF_H   0xFD

#define PWM0_ON_L   0x06
#define PWM0_ON_H   0x07
#define PWM0_OFF_L  0x08
#define PWM0_OFF_H  0x09
#define PWM1_ON_L   0x0A
#define PWM1_ON_H   0x0B
#define PWM1_OFF_L  0x0C
#define PWM1_OFF_H  0x0D
#define PWM2_ON_L   0x0E
#define PWM2_ON_H   0x0F
#define PWM2_OFF_H  0x10
#define PWM2_OFF_L  0x11
#define PWM3_ON_L   0x12
#define PWM3_ON_H   0x13
#define PWM3_OFF_L  0x14
#define PWM3_OFF_H  0x15
#define PWM4_ON_L   0x16
#define PWM4_ON_H   0x17
#define PWM4_OFF_L  0x18
#define PWM4_OFF_H  0x19
#define PWM5_ON_L   0x1A
#define PWM5_ON_H   0x1B
#define PWM5_OFF_L  0x1C
#define PWM5_OFF_H  0x1D
#define PWM6_ON_L   0x1E
#define PWM6_ON_H   0x1F
#define PWM6_OFF_L  0x20
#define PWM6_OFF_H  0x21
#define PWM7_ON_L   0x22
#define PWM7_ON_H   0x23
#define PWM7_OFF_L  0x24
#define PWM7_OFF_H  0x25
#define PWM8_ON_L   0x26
#define PWM8_ON_H   0x27
#define PWM8_OFF_L  0x28
#define PWM8_OFF_H  0x29
#define PWM9_ON_L   0x2A
#define PWM9_ON_H   0x2B
#define PWM9_OFF_L  0x2C
#define PWM9_OFF_H  0x2D
#define PWM10_ON_L  0x2E
#define PWM10_ON_H  0x2F
#define PWM10_OFF_L 0x30
#define PWM10_OFF_H 0x31
#define PWM11_ON_L  0x32
#define PWM11_ON_H  0x33
#define PWM11_OFF_L 0x34
#define PWM11_OFF_H 0x35
#define PWM12_ON_L  0x36
#define PWM12_ON_H  0x37
#define PWM12_OFF_L 0x38
#define PWM12_OFF_H 0x39
#define PWM13_ON_L  0x3A
#define PWM13_ON_H  0x3B
#define PWM13_OFF_L 0x3C
#define PWM13_OFF_H 0x3D
#define PWM14_ON_L  0x3E
#define PWM14_ON_H  0x3F
#define PWM14_OFF_L 0x40
#define PWM14_OFF_H 0x41
#define PWM15_ON_L  0x42
#define PWM15_ON_H  0x43
#define PWM15_OFF_L 0x44
#define PWM15_OFF_H 0x45



namespace pca9685 {
    void init();
    void destroy();

    

    void wake_up();
    void restart();
    void sleep();

    void set_frequency(int rate);
    int get_frequency();

    void set_pwm_on(int pwm, int on);
    void set_pwm_off(int pwm, int off);

    void set_pwm_percent(int pwm, double percent);
    void set_pwm_ms(int pwm, int ms);


    // Set all pwms on time delays
    void set_all_pwm_on();
    // Set all pwms off time delays
    void set_all_pwm_off();

    // Set all pwms percent of full
    void set_all_pwm_percent();
    // Set all pwms percent of 
    void set_all_pwm_ms();

    int query_reg(int reg);
    void write_reg(int reg, int val);
};

#endif