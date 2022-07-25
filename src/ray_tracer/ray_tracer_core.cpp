#include "ray_tracer/ray_tracer_core.h"


void as::test()
{

}

bool as::create_camera(camera* out_camera, const camera_create_info& create_info)
{
	assert(out_camera);

	f32 theta = glm::radians(create_info.vfov);
	f32 h = tan(theta / 2);
	f32 viewport_height = 2.0 * h;
	f32 viewport_width = create_info.aspect_ratio * viewport_height;

	out_camera->w = glm::normalize(create_info.look_from - create_info.look_at);
	out_camera->u = glm::normalize(glm::cross(create_info.vup, out_camera->w)); // TODO : make sure Normalize and unit vec are the same
	out_camera->v = glm::cross(out_camera->w, out_camera->u);

	out_camera->origin = create_info.look_from;
	out_camera->horizontal = create_info.focus_dist * viewport_width * out_camera->u;
	out_camera->vertical = create_info.focus_dist * viewport_height * out_camera->v;
	out_camera->lower_left_corner = out_camera->origin - out_camera->horizontal / 2.f - out_camera->vertical / 2.f - create_info.focus_dist * out_camera->w;

	out_camera->lens_radius = create_info.aperture / 2;
	out_camera->time = create_info.time;
	return true;
}
