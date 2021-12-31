#pragma once

#include "vec3.h"

class ray
{

public:

    ray(){};
    ray(const point3& origin, const vec3& direction, const double time = .0) : orig(origin), dir(direction), tm(time){};

    inline point3 origin() const {return orig;}
    inline vec3 direction() const {return dir;}
    inline double time() const {return tm;}

    inline point3 at(double t) const
    {
        return orig + t*dir;
    }

public:

    point3 orig;
    vec3 dir;
    double tm;
};