#pragma once

#include <RayTracer/utility.h>
#include <RayTracer/common.h>
#include <RayTracer/hittable.h>

struct hit_record;

class material
{

public:

    material(/* args */);
    ~material();

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;

private:

    /* data */
};

class lambertian : public material
{

public:

    lambertian(const color& a) : albedo(a) {};

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override;

public:

    color albedo;

};

class metal : public material
{

public:

    metal(const color& a, double f) : albedo(a), fuzz(f < 1. ? f : 1.f) {};

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override;

public:

    color albedo;
    double fuzz;

};

class dielectric : public material
{

public:

    dielectric(double index_of_refraction) : ir(index_of_refraction) {};

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override;

private:

    static double reflectance(const double cosine, const double ref_idx);

public:

    double ir; // Index of Refraction

};