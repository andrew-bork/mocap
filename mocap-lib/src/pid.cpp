#include <pid.h>
#include <algorithm>

pid::pid(){};

pid::pid(double p_p, double p_i, double p_d){
    kP = p_p;
    kI = p_i;
    kD = p_d;
};

double pid::update(double curr, double dt){
    err = setpoint - curr;


    if(err < i_max_err && err > -i_max_err){
        i_curr += dt * err;
        i_curr = std::max(std::min(i_curr, i_max), -i_max);
    }

    derr = (err - old_error) / dt;  
    old_error = err;

    p = kP * err;
    i = kI * i_curr;
    d = kD * derr;

    output = p + i + d;
    return output;
    // return i_curr * kI + kP * err - kD * derr;
}

void pid::reset_integral_term(){
    i_curr = 0.0;
}

void pid::change_setpoint(double new_setpoint){
    setpoint = new_setpoint;
    // i_curr = 0.0;
}