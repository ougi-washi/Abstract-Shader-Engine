#pragma once

#include <cmath>
#include <iostream>
#include <RayTracer/utility.h>

class vec3
{

public:

    vec3() : e{0, 0, 0} {}
    vec3(const double e) : e{e, e, e} {};
    vec3(const double e0, const double e1, const double e2) : e{e0, e1, e2} {};

    inline double x() const { return e[0]; }
    inline double y() const { return e[1]; }
    inline double z() const { return e[2]; }

    inline double r() const { return e[0]; }
    inline double g() const { return e[1]; }
    inline double b() const { return e[2]; }

    inline const vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); };
    inline const double operator[](int i) const { return e[i]; };
    inline const double& operator[](int i) { return e[i]; };

    inline vec3& operator+=(const vec3 &v)
    {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }

    inline vec3& operator*=(const double t)
    {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    inline vec3& operator/=(const double t)
    {
        return *this *= 1/t;
    }

    inline double length_squared() const 
    {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }

    inline double length() const 
    {
        return std::sqrt(length_squared());
    }

    inline static vec3 random()
    {
        return vec3(random_double(), random_double(), random_double());
    }

    inline static vec3 random(const double min, const double max)
    {
        return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
    }    

    inline bool near_zero() const
    {
        // Return true if the vector is close to zero in all dimensions
        const auto s = 1e-8;
        return (fabs(e[0] < s) && fabs(e[1] < s) && fabs(e[2] < s));
    }

public:
    double e[3];

};

// Aliases
using point3 = vec3;
using color = vec3;


inline std::ostream& operator<<(std::ostream &out, const vec3 &v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(const double t, const vec3 &v) {
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3 &v, const double t) {
    return t * v;
}

inline vec3 operator/(const vec3 &v, const double t) {
    return v * (1 / t);
}

inline double dot(const vec3 &v, const vec3 &u) {
    return u.e[0] * v.e[0]
        + u.e[1] * v.e[1]
        + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3 &u, const vec3 &v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3 v)
{
    return v / v.length();
}

inline vec3 random_in_unit_sphere()
{
    while (true)
    {
        auto p = vec3::random(-1., 1.);
        if (p.length_squared() >= 1)
        {
            continue;
        }
        return p;
    }
    return vec3(.5, .5, .5);
}

inline vec3 random_unit_vector()
{
    return unit_vector(random_in_unit_sphere());
}

inline vec3 random_in_hemisphere(const vec3& normal)
{
    vec3 in_unit_sphere = random_in_unit_sphere();
    if (dot(in_unit_sphere, normal) > 0.) // In the same hemisphere as the normal
    {
        return in_unit_sphere;
    }
    else
    {
        return -in_unit_sphere;
    }
}

inline vec3 random_in_unit_disk()
{
    while (true)
    {
       vec3 p = vec3(random_double(-1., 1.), random_double(-1., 1.), 0.);
       if (p.length_squared() >= 1.)
       {
           continue;
       }
       return p;
    }
}

inline vec3 reflect(const vec3& v, const vec3& n)
{
    return v - 2*dot(v, n) * n;
}

inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat)
{
    auto cos_theta = fmin(dot(-uv, n), 1.);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -sqrt(fabs(1. - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}