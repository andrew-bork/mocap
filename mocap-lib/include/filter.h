#ifndef FILTER_H_GUARD
#define FILTER_H_GUARD

namespace filter {
    struct filter {
        double a1=0, a2=0, b0=1, b1=0, b2=0;
        double x1=0, x2=0, y1=0, y2=0;
        double operator [](double inp);
    };
    
    filter low_pass(double samp_rate, double w0);
    filter band_pass(double samp_rate, double wL, double wU);
    filter high_pass(double samp_rate, double w0);
    filter none();
    // filter complimentary(double samp_rate);
};
#endif