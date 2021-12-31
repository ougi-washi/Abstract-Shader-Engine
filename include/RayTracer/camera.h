#pragma once

#include <RayTracer/utility.h>
#include <RayTracer/common.h>

class camera
{

public:

    camera();

    camera( const point3 look_from, const point3 look_at, const vec3 vup, 
            const double vfov, const double aspect_ratio, const double aperture,
            const double focus_dist, const double in_time0 = 0, const double in_time1 = 0);

    ray get_ray(const double s, const double t) const;

private:

    point3 origin;

    point3 lower_left_corner;

    vec3 horizontal, vertical;

    vec3 u, v, w;

    double time0, time1; // Shutter open/close time

    double lens_radius;
};