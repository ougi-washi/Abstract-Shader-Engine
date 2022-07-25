#pragma once
#include "ray_tracer/ray_tracer.h"

namespace as
{
	void test();
	
	/** Camera */
	bool create_camera(camera* out_camera, const camera_create_info& create_info);
	//ray get_ray_from_camera(const f32& , );

};