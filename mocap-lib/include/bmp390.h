#ifndef bmp390_h
#define bmp390_h

#define BMP390_ADDR 0x77

// SETTINGS
//                    00010011
#define BMP390_MODE 0b00110011 //NORMAL, PRESSURE ENABLED, TEMPERATURE ENABLED
#define BMP390_RESOLUTION // Standard
#define BMP390_OSRS 0b00000011 //x8 PRESSURE, x1 TEMPERATURE
#define BMP390_CONFIG 0b00000010 //2 IIR FILTER

#define BMP390_SOFT_RESET 0xB6

// SETTINGS REG
#define BMP390_REG_PWR_CTRL 0x1B
#define BMP390_REG_OSR 0x1C
#define BMP390_REG_CONFIG 0x1F
#define BMP390_REG_PRESS_7_0 0x04
#define BMP390_REG_PRESS_15_8 0x05
#define BMP390_REG_PRESS_23_16 0x06
#define BMP390_REG_TEMP_7_0 0x07
#define BMP390_REG_TEMP_15_8 0x08
#define BMP390_REG_TEMP_23_16 0x09
#define BMP390_REG_CMD 0x7E // Commands
#define BMP390_REG_OSR 0x1C // Oversampling Settings
#define BMP390_REG_ODR 0x1D // Output Data Rate

//TRIMMING MEM
#define NVM_PAR_P11 0x45
#define NVM_PAR_P10 0x44
#define NVM_PAR_P9_H 0x43
#define NVM_PAR_P9_L 0x42
#define NVM_PAR_P8 0x41
#define NVM_PAR_P7 0x40
#define NVM_PAR_P6_H 0x3F
#define NVM_PAR_P6_L 0x3E
#define NVM_PAR_P5_H 0x3D
#define NVM_PAR_P5_L 0x3C
#define NVM_PAR_P4 0x3B
#define NVM_PAR_P3 0x3A
#define NVM_PAR_P2_H 0x39
#define NVM_PAR_P2_L 0x38
#define NVM_PAR_P1_H 0x37
#define NVM_PAR_P1_L 0x36
#define NVM_PAR_T3 0x35
#define NVM_PAR_T2_H 0x34
#define NVM_PAR_T2_L 0x33
#define NVM_PAR_T1_H 0x32
#define NVM_PAR_T1_L 0x31

#define BMP390_PRES_ENABLE 0b1
#define BMP390_TEMP_ENABLE 0b10
#define BMP390_NORM_MODE 0x30
#define BMP390_FORCE_MODE 0x10
#define BMP390_SLEEP_MODE 0x00
namespace bmp390 {

    enum pwr {
        SLEEP = 0b00, NORMAL = 0b11, FORCED = 0b10
    };
    
    enum oversampling {
        ULTRA_LOW_POWER = 0b000,
        LOW_POWER = 0b001,
        STANDARD = 0b010,
        HIGH = 0b011,
        ULTRA_HIGH = 0b100,
        HIGHEST = 0b101
    };

    enum iir_filter {
        COEFF_0 = 0b000,
        COEFF_1 = 0b001,
        COEFF_3 = 0b010,
        COEFF_7 = 0b011,
        COEFF_15 = 0b100,
        COEFF_31 = 0b101,
        COEFF_63 = 0b110,
        COEFF_127 = 0b111
    };

    enum output_data_rate {
        hz200 = 0x00, // 200 hz
        hz100 = 0x01,
        hz50 = 0x02,
        hz25 = 0x03,
        hz25_2 = 0x04,
        hz25_4 = 0x05,
        hz25_8 = 0x06,
        hz25_16 = 0x07,
        hz25_32 = 0x08,
        hz25_64 = 0x09,
        hz25_128 = 0x0A,
        hz25_256 = 0x0B,
        hz25_512 = 0x0C,
        hz25_1024 = 0x0D,
        hz25_2048 = 0x0E,
        hz25_4096 = 0x0F,
        hz25_8192 = 0x10,
        hz25_16384 = 0x11,
    };

    void init();

    void acquire_calib_vars();

    int get_raw_temp();
    double get_temp();
    int get_raw_press();
    double get_press();
    double get_press(double temp);
    double get_height();
    double get_height(double temp, double press);
    void get_data(double * data);


    void set_iir_filter(iir_filter coeff);
    void set_oversample(oversampling pressure, oversampling temperature);
    void set_pressure_oversample(oversampling pressure);
    void set_temperature_oversample(oversampling temperature);
    void set_output_data_rate(output_data_rate rate);
    void set_pwr_mode(pwr mode);
    void set_enable_pressure(bool enabled);
    void set_enable_temperature(bool enabled);
    void set_enable(bool pressure, bool temperature);
    void set_pwr_ctrl(int val);

    int chk_error();
    int print_error();

    int query_register(int reg);

    void soft_reset();

    void stop();

    void print_compensations();
};










#endif