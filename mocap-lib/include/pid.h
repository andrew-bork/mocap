#ifndef PID_H
#define PID_H

#define I_MAX_ERR 1.0
#define I_MAX 1.0

struct pid {

    double kP = 0.0;
    double kI = 0.0;
    double kD = 0.0;

    double p = 0.0;
    double i = 0.0;
    double d = 0.0;

    double i_curr = 0.0;
    double setpoint = 0.0;
    double old_error = 0.0;

    double err = 0.0;
    double derr = 0.0;

    double i_max = I_MAX;
    double i_max_err = I_MAX_ERR;

    double max_pwr = 1.0;
    double min_pwr = 0.0;

    double output = 0.0;

    pid();
    pid(double p_p, double p_i, double p_d);


    double update(double curr, double dt);
    void change_setpoint(double new_setpoint);

    void reset_integral_term();
};

#endif















