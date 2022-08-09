#pragma once
#include "ray_tracer/ray_tracer.h"
#include <iostream> // TODO: I HATE THIS, to remove

namespace as
{
	/** camera */
	bool create_camera(camera* out_camera, const camera_create_info& create_info);
	void get_ray_from_camera(ray* out_ray, const camera* in_camera, const location2D& coord);

	/** ray */
	direction reflect(const direction& base_dir, const direction& normal);
	location get_location_by_ray_distance(const ray* in_ray, const f32& in_distance);
	color get_ray_color(const ray& in_ray, const world* in_world, const u16& depth);
	void set_hit_record_normal(hit_record* out_hit_record, const ray& in_ray, const direction& outward_normal);
	bool hit(hit_record* out_hit_record, const world* in_world, const ray& in_ray, const glm::vec2& range);
	bool hit(hit_record* out_hit_record, const sphere* in_sphere, const ray& in_ray, const glm::vec2& range);
	bool scatter(ray* out_scattered_ray, color* out_attenuation, const ray* in_ray, const hit_record& in_hit_record);

	/** time stamp */
	float get_random_from_time_stamp(const time_stamp& in_time_stamp);

	/** file */

	void write_color_ppm(std::ostream& out, color pixel_color, i32 samples_per_pixel);

	/** test */
	void ray_tracer_test();

};