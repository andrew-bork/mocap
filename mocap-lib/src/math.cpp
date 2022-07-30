#include <math.h>
#include <cmath>


math::quarternion::quarternion(){
    w = x = y = z = 0;
    unit = false;
}

math::quarternion::quarternion(double x, double y, double z){
    w = 0;
    this->x = x;
    this->y = y;
    this->z = z;
    unit = false;
}
math::quarternion::quarternion(double w, double x, double y, double z){
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
    unit = false;
}

math::quarternion::quarternion(double w, double x, double y, double z, bool unit){
    this->w = w;
    this->x = x;
    this->y = y;
    this->z = z;
    this->unit = unit;
}



math::quarternion math::quarternion::operator*(const math::quarternion& r){
    quarternion res;
    res.w = w*r.w - x*r.x - y*r.y - z*r.z;
    res.x = r.w*x + r.x*w + r.y*z - r.z*y;
    res.y = r.w*y + r.y*w + r.z*x - r.x*z;
    res.z = r.w*z + r.z*w + r.x*y - r.y*x;
    return res;
}

math::quarternion math::quarternion::operator+(const math::quarternion& r){
    quarternion res;
    res.w = r.w + w;
    res.x = r.x + x;
    res.y = r.y + y;
    res.z = r.z + z;
    return res;
    
}

math::quarternion math::quarternion::conjugate(const math::quarternion& q){
    quarternion res;
    res.w = q.w;
    res.x = -q.x;
    res.y = -q.y;
    res.z = -q.z;
    return res;
    
}

double math::length(const math::quarternion& q){
    return sqrt(q.w*q.w+q.x*q.x+q.y*q.y+q.z*q.z);
}

double math::length(const math::vector& v){
    return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

math::quarternion math::quarternion::inverse(const math::quarternion& q){
    double len;
    if(q.unit) len = 1;
    else len = length(q);
    quarternion res;
    res.w = q.w;
    res.x = -q.x;
    res.y = -q.y;
    res.z = -q.z;
    return res;
}
math::quarternion math::quarternion::rotate(double theta, const math::vector& axis){
    quarternion res;
    res.w = cos(theta/2);
    res.x = axis.x*sin(theta/2);
    res.y = axis.y*sin(theta/2);
    res.z = axis.z*sin(theta/2);
    return res;
}

math::quarternion math::quarternion::fromEulerZYX(const math::vector& euler){

    double cy = cos(euler.z*0.5);
    double sy = sin(euler.z*0.5);
    double cp = cos(euler.y*0.5);
    double sp = sin(euler.y*0.5);
    double cr = cos(euler.x*0.5);
    double sr = sin(euler.x*0.5);

    quarternion res;
    res.w = cr*cp*cy + sr*sp*sy;
    res.x = sr*cp*cy - cr*sp*sy;
    res.y = cr*sp*cy + sr*cp*sy;
    res.z = cr*cp*sy - sr*sp*cy;
    return res;
}

math::vector math::quarternion::toEuler(const math::quarternion& q){
    double  sin_p = 2*(q.w*q.y - q.z*q.x);
    if(sin_p >=1){
    vector res(-2*atan2(q.x, q.w), 1.570796326794897, 0);
    return res;
    }else if(sin_p <= -1){
    vector res(2 *atan2(q.x, q.w), -1.570796326794897,  0);
    return res;
    }
    //asin(2*(q.w*q.y - q.z*q.x));
    vector res(atan2(2*(q.w*q.x+q.y*q.z), 1 - 2* (q.x*q.x + q.y*q.y)), asin(sin_p), atan2(2*(q.w*q.z+q.x*q.y), 1 - 2 * (q.y*q.y+q.z*q.z)));
    return res;
}
/*



    EulerAngles angles;

    // roll (x-axis rotation)
    double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    angles.roll = std::atan2(sinr_cosp, cosr_cosp);

    // pitch (y-axis rotation)
    double sinp = 2 * (q.w * q.y - q.z * q.x);
    if (std::abs(sinp) >= 1)
        angles.pitch = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        angles.pitch = std::asin(sinp);

    // yaw (z-axis rotation)
    double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    angles.yaw = std::atan2(siny_cosp, cosy_cosp);

    return angles;



*/
math::vector math::quarternion::toMagAxis(const math::quarternion& q){
    double s = 1 - q.w*q.w;
    double mag = acos(q.w)*2;

    if(q.w > -0.00001 && q.w < 0.00001){
        vector res(0,0,0);
        return res;
    }
    vector res(q.x*mag/s, q.y*mag/s,q.z*mag/s);
    return res;
}

math::vector::vector (){
    x = y = z = 0;
}
math::vector::vector (double xa){
    x = xa;
    y = z = 0;
}
math::vector::vector (double xa, double ya){
    x = xa;
    y = ya;
    z = 0;
}
math::vector::vector (double xa, double ya, double za){
    x = xa;
    y = ya;
    z = za;
}

math::vector math::vector::operator+ (const math::vector& r){
    vector res(x+r.x,y+r.y,z+r.z);
    return res;
}

math::vector math::vector::operator*(const double& s){
    vector res(x*s, y*s, z*s);
    return res;
}

math::vector math::quarternion::rotateVector(math::quarternion& q, math::vector& r){
    quarternion temp(0, r.x, r.y, r.z);
    temp = q * temp * quarternion::conjugate(q);
    vector out = math::vector(temp.x, temp.y, temp.z);
    return out;
}