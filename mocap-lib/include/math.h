#ifndef MATH_H
#define MATH_H

#define DEG_TO_RAD 0.017453292519943
#define RAD_TO_DEG 57.29577951308232

namespace math{

    struct vector{
        double x,y,z;
        vector();
        vector(double x);
        vector(double x, double y);
        vector(double x, double y, double z);

        vector operator+ (const vector& r);
        vector operator* (const double& s);
    };

    struct quarternion{
        double w, x, y, z;
        bool unit;

        quarternion();
        quarternion(double x, double y, double z);
        quarternion(double w, double x, double y, double z);
        quarternion(double w, double x, double y, double z, bool unit);

        quarternion operator+ (const quarternion& r);
        quarternion operator* (const quarternion& r);

        static quarternion inverse(const quarternion& n);
        static quarternion conjugate(const quarternion& n);
        static quarternion rotate(double theta, const vector& axis);
        static quarternion fromEulerZYX(const vector& euler);
        
        static vector toEuler(const quarternion& q);
        static vector toMagAxis(const quarternion& q);
        static vector rotateVector(math::quarternion& q, math::vector& in);
    };
    double length(const quarternion& n);
    double length(const vector& n);
}
#endif