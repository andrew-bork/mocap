#ifndef DRONE_H
#define DRONE_H
#include <settings.h>
#include <pwm_s.h>
#include <unistd.h>

namespace drone {
    pwm fl, fr, bl, br;













    drone();


    /**
     * DRONE SETTINGS
     * */
    void init();
    void enable(bool fl, bool fr, bool bl, bool br);
    void destoy();

    /**
     * MOTOR SETTINGS
     * */
    void set_all(int width);
    void set_diagonals(short diagonal, int width);
    void set_motor(short motor, int width);

    
    /**
     * ROUTINES
     * */
    void arm();


};




#endif