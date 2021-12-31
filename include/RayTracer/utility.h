#pragma once

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degree_to_redians(const double degrees)
{
    return degrees * pi / 180.;
}

inline double random_double()
{
    // Returns a random real in [0, 1)
    return rand() / (RAND_MAX + 0.f);
}

inline double random_double(const double min, const double max)
{
    // Returns a random real in [min/max)
    return min + (max - min) * random_double();
}

inline double clamp(const double x, const double min, const double max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}