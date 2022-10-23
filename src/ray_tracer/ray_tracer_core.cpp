#include "ray_tracer/ray_tracer_core.h"

/** GENERAL */
#include <float.h> 
/** GLM */
#include "glm/gtx/optimum_pow.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/norm.hpp"

const double pi = 3.1415926535897932385;

glm::vec3 unit_vector(glm::vec3 v) {
	return v / glm::length(v);
}

bool as::create_camera(camera* out_camera, const camera_create_info& create_info)
{
	assert(out_camera);

	//f32 theta = glm::radians(create_info.vfov); TODO
	f32 theta = create_info.vfov * pi / 180.0;
	f32 h = tan(theta / 2);
	f32 viewport_height = 2.0 * h;
	f32 viewport_width = create_info.aspect_ratio * viewport_height;

	//out_camera->w = glm::normalize(create_info.look_from - create_info.look_at); TODO
	out_camera->w = unit_vector(create_info.look_from - create_info.look_at);
	//out_camera->u = glm::normalize(glm::cross(create_info.vup, out_camera->w)); // TODO: make sure Normalize and unit vec are the same
	out_camera->u = unit_vector(glm::cross(create_info.vup, out_camera->w));
	out_camera->v = glm::cross(out_camera->w, out_camera->u);

	out_camera->origin = create_info.look_from;
	out_camera->horizontal = create_info.focus_dist * viewport_width * out_camera->u;
	out_camera->vertical = create_info.focus_dist * viewport_height * out_camera->v;
	out_camera->lower_left_corner = out_camera->origin - out_camera->horizontal / 2.f - out_camera->vertical / 2.f - create_info.focus_dist * out_camera->w;

	out_camera->lens_radius = create_info.aperture / 2;
	out_camera->time = create_info.time;
	return true;
}

void as::get_ray_from_camera(ray* out_ray, const camera* in_camera, const location2D& coord)
{
	assert(out_ray);
	assert(in_camera);
	
	location random_disk = in_camera->lens_radius * location(glm::diskRand(1.f), 0.f); // TODO: make sure diskRand is the right function
	location offset = location(coord.x * random_disk.x + coord.y * random_disk.y);
	
	out_ray->loc = in_camera->origin + offset;
	out_ray->dir = in_camera->lower_left_corner + coord.x * in_camera->horizontal + coord.y * in_camera->vertical - in_camera->origin - offset;
	out_ray->distance = get_random_from_time_stamp(in_camera->time);
}

as::direction as::reflect(const direction& base_dir, const direction& normal)
{
	return base_dir - 2 * glm::dot(base_dir, normal) * normal;
}

as::location as::get_location_by_ray_distance(const ray* in_ray, const f32& in_distance)
{
	assert(in_ray);
	return in_ray->loc + in_ray->distance * in_ray->dir;
}

as::color as::get_ray_color(const ray& in_ray, const world* in_world, const u16& depth)
{
	if (depth <= 0)
	{
		return color(.0, .0, .0);
	}
	hit_record record;
	glm::vec2 range(0.001f, FLT_MAX);
	if (hit(&record, in_world, in_ray, range))
	{
		ray scatted_ray;
		color attenuation;
		if (scatter(&scatted_ray, &attenuation, &in_ray, record))
		{
			return attenuation * get_ray_color(scatted_ray, in_world, depth - 1);
		}
		else
		{
			return color(.0f);
		}
	}
	direction unit_direction = glm::normalize(in_ray.dir);
	f32 distance = .5f * (unit_direction.y + 1.);
	return (1.f - distance) * color(1.f) + distance * color(.5f, .7f, .1f);
}

void as::set_hit_record_normal(hit_record* out_hit_record, const ray& in_ray, const direction& outward_normal)
{
	assert(out_hit_record);

	out_hit_record->front_face = glm::dot(in_ray.dir, outward_normal) < 0.f;
	out_hit_record->normal = out_hit_record->front_face ? outward_normal : -outward_normal;
}

bool as::hit(hit_record* out_hit_record, const world* in_world, const ray& in_ray, const glm::vec2& range)
{
	assert(out_hit_record);
	assert(in_world);

	//hit_record temp_hit_record;
	glm::vec2 new_range = range;
	bool hit_anything = false;
	for (u16 i = 0 ; i < in_world->sphere_count ; i++)
	{
		if (hit(out_hit_record, &in_world->spheres[i], in_ray, new_range))
		{
			hit_anything = true;
			new_range.y = out_hit_record->time;
			//memcpy(out_hit_record, &temp_hit_record, sizeof(out_hit_record));
		}
	}
	return hit_anything;
}

bool as::scatter(ray* out_scattered_ray, color* out_attenuation, const ray* in_ray, const hit_record& in_hit_record)
{
	assert(out_scattered_ray);
	assert(out_attenuation);
	assert(in_hit_record.mat);
	assert(in_ray);

	direction reflected_direction = reflect(glm::normalize(in_ray->dir), in_hit_record.normal);
	out_scattered_ray->loc = in_hit_record.loc;
	out_scattered_ray->dir = in_hit_record.mat->metal * glm::sphericalRand(1.f);
	out_scattered_ray->distance = in_ray->distance;
	*out_attenuation = in_hit_record.mat->albedo;
	return (glm::dot(out_scattered_ray->dir, in_hit_record.normal) > 0.f);
}

bool as::hit(hit_record* out_hit_record, const sphere* in_sphere, const ray& in_ray, const glm::vec2& range)
{
	assert(out_hit_record);
	assert(in_sphere);

	location origin = in_ray.loc - in_sphere->center; // TODO: for moving spheres, change this later
	f32 a = glm::length2(in_ray.dir);
	f32 half_b = glm::dot(origin, in_ray.dir);
	f32 c = glm::length2(origin) - glm::pow2(in_sphere->radius);
	f32 discriminant = half_b * half_b - a * c;
	if (discriminant < 0.f)
	{
		return false;
	}

	f32 sqrtd = glm::sqrt(discriminant);
	// Find the nearest root that lies in the acceptable range
	f32 root = (-half_b - sqrtd) / a;
	if (root < range.x || range.y < root)
	{
		root = (-half_b + sqrtd) / a;
		if (root < range.x || range.y < root)
		{
			return false;
		}
	}
	out_hit_record->time = root;
	out_hit_record->loc = get_location_by_ray_distance(&in_ray, out_hit_record->time);// in_ray.at(rec.t);
	set_hit_record_normal(out_hit_record, in_ray, (out_hit_record->loc - in_sphere->center) / in_sphere->radius);
	out_hit_record->mat = in_sphere->mat;
	return true;
}

float as::get_random_from_time_stamp(const time_stamp& in_time_stamp)
{
	// This will always generate same values, in order to change it, the seed has to be different from the default
	return glm::linearRand(in_time_stamp.start, in_time_stamp.end);
}

void as::write_color_ppm(std::ostream& out, color pixel_color, i32 samples_per_pixel)
{
	f32 r = pixel_color.x;
	f32 g = pixel_color.y;
	f32 b = pixel_color.z;

	//Divide the color by the number of samples and gamma-correct for gamma=2.0
	auto scale = 1. / samples_per_pixel;
	r = glm::sqrt(scale * r);
	g = glm::sqrt(scale * g);
	b = glm::sqrt(scale * b);

	// Write the translated [0, 255] value of each color component.
	out << static_cast<int>(256 * glm::clamp(r, 0.f, 0.999f)) << ' '
		<< static_cast<int>(256 * glm::clamp(g, 0.f, 0.999f)) << ' '
		<< static_cast<int>(256 * glm::clamp(b, 0.f, 0.999f)) << '\n';
}

void as::ray_tracer_test()
{
	/** camera */
	camera main_camera = {};
	camera_create_info main_camera_create_info = {};
	main_camera_create_info.look_from = location(0.f, 0.f, 2.f);
	main_camera_create_info.look_at = location(0.f, 0.f, 0.f);
	main_camera_create_info.vup = location(0.f, 1.f, 0.f);
	main_camera_create_info.focus_dist = 1.f;
	main_camera_create_info.aperture = .1f;
	create_camera(&main_camera, main_camera_create_info);

	/** material */
	material metallic_material = {};
	metallic_material.albedo = color(1.f, 0.f, 0.f);
	metallic_material.metal = .5f;

	/** scene */
	world main_world = {};
	const u16 sphere_count = 1;

	main_world.sphere_count = sphere_count;
	main_world.spheres = (sphere*)malloc(sizeof(sphere) * sphere_count);

	main_world.spheres[0].center = location(0.f, 0.f, 0.f);
	main_world.spheres[0].radius = 1.f;
	main_world.spheres[0].mat = &metallic_material;

	/** render */

	const f32 aspect_ratio = 16.f / 9.f;
	const i32 image_width = 400;
	const i32 image_height = static_cast<i32>(image_width / aspect_ratio);
	const i32 samples_per_pixel = 100;
	const i32 max_depth = 50;

	// Write to a file
	freopen("output.ppm", "w", stdout);
	std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	for (int j = image_height - 1; j >= 0; --j)
	{
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			color pixel_color(0, 0, 0);
//#pragma omp parallel for
			for (int s = 0; s < samples_per_pixel; s++)
			{
				// Set up UV
				f32 u = f32(i + rand()) / (image_width - 1.);
				f32 v = f32(j + rand()) / (image_height - 1.);

				ray current_ray; 
				get_ray_from_camera(&current_ray, &main_camera, glm::vec2(u, v));

				color result_pixel_color = get_ray_color(current_ray, &main_world, max_depth);
//#pragma omp critical
				{
					pixel_color += result_pixel_color;
				}
			}
			write_color_ppm(std::cout, pixel_color, samples_per_pixel);
		}
	}
	std::cerr << "\nDone.\n";
}
