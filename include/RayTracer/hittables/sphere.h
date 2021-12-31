#pragma once 

#include <RayTracer/hittable.h>
#include <RayTracer/material.h>

class sphere : public hittable
{

public:

    sphere(){};
    
    sphere(const point3 cen, const double r, shared_ptr<material> m) : center(cen), radius(r), mat_ptr(m) {};
    
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

public:

    point3 center;
    double radius;
    shared_ptr<material> mat_ptr;

};

class moving_sphere : public hittable
{

public:

    moving_sphere(){};
    
    moving_sphere(  const point3 cen0, const point3 cen1, const double _time0, const double _time1, 
                    const double r, shared_ptr<material> m) : 
                    center0(cen0), center1(cen1), time0(_time0), time1(_time1), radius(r), mat_ptr(m) {};
    
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

    point3 center(const double time) const;

public:

    point3 center0 , center1;
    double radius;
    shared_ptr<material> mat_ptr;
    double time0, time1;
};