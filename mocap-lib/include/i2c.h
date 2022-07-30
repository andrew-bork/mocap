#ifndef I2C_GUARD
#define I2C_GUARD

namespace i2c {
    int read(int fd, int reg);
    int write(int fd, int reg, int data);
}

#endif

