#include <pid.h>
#include <algorithm>

pid::pid(){};

pid::pid(double p_p, double p_i, double p_d){
    p = p_p;
    i = p_i;
    d = p_d;
};

double pid::output(double curr, double dt){
    double err = setpoint - curr;


    if(err < I_MAX_ERR && err > -I_MAX_ERR){
        i_curr += dt * err;
        i_curr = std::max(std::min(i_curr, I_MAX), -I_MAX);
    }

    double derr = (err - old_error) / dt;  
    old_error = err;
    return i_curr * i + p * err - d * derr;
}

void pid::reset_integral_term(){
    i_curr = 0.0;
}

void pid::change_setpoint(double new_setpoint){
    setpoint = new_setpoint;
    i_curr = 0.0;
}