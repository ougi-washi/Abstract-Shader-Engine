#include <RayTracer/render.h>
#include <RayTracer/utility.h>
#include <RayTracer/hittables/sphere.h>
#include <RayTracer/color.h>
#include <RayTracer/common.h>

#include <omp.h> // TODO : READ https://pvs-studio.com/en/blog/posts/cpp/a0054/

render::render() 
{
}

void render::init() 
{
    // camera
    point3 look_from(13., 2., 3.);
    point3 look_at(0., 0., 0.);
    vec3 vup(0., 1., 0.);
    double dist_to_focus = 10.;
    double aperture = .1;
    cam = camera(look_from, look_at, vup, 20., aspect_ratio, aperture, dist_to_focus, 0., 1.);

    // Scene
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3( 0.0, -1000., 0.), 1000.0, ground_material));

    for (int a = -11; a < 11; a++) 
    {
        for (int b = -11; b < 11; b++) 
        {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) 
            {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.2) 
                {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    // auto center2 = center + vec3(0, random_double(0, 1), 0);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else if (choose_mat < 0.95)
                {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else 
                {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    // Write to a file
    freopen("output.ppm", "w", stdout);

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
}

void render::process()
{
    for (int j = image_height-1; j >= 0; --j) 
    {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) 
        {
            color pixel_color(0, 0, 0);
            #pragma omp parallel for
            for (int s =0 ; s < samples_per_pixel ; s++)
            {
                // Set up UV
                double u = double(i + random_double()) / (image_width - 1.);
                double v = double(j + random_double()) / (image_height - 1.);
                
                ray r = cam.get_ray(u, v);
                color result_pixel_color = ray_color(r, world, max_depth);
                #pragma omp critical
                {
                    pixel_color += result_pixel_color;
                }
            }
            write_color_ppm(std::cout, pixel_color, samples_per_pixel);
        }
    }
    std::cerr << "\nDone.\n";
}

color render::ray_color(const ray& r, const hittable& world, const int depth) 
{
    if (depth <= 0)
    {
        return color(.0, .0, .0);
    }

    hit_record rec;
    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        color attenuation;
        if(rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * ray_color(scattered, world, depth-1);
        }
        else 
        {
            return color(0.);
        }
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t  = .5 * (unit_direction.y() + 1.);
    return (1.-t) * color(1., 1., 1.) + t * color(.5, .7, 1.);
}