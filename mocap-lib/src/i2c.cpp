#include <i2c.h>
#include <unistd.h>

int _read(int fd, char * buf, int l) {
    return read(fd, buf, l);
}

int _write(int fd, char * buf, int l) {
    return write(fd, buf, l);
}

int i2c::read(int fd, int reg) {
    return _read(fd, buf, 1);
}

int i2c::write(int fd, int reg, int data) {
    char buf = (char) data;
    return _write(fd, &buf, 1);
}