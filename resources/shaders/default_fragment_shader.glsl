#include "resources/shaders/common/fragment_shader_common.glsl"

void main()
{
	// ambient color
	vec3 final_color = vec3(0.03);

	// first ambient pass for lights
	for(int i = 0 ; i < lights_count ; i++)
	{ 
        // Ambient lighting
		final_color += calculate_light(lights[i]);
	}
	
	// rim light
	for(int i = 0 ; i < lights_count ; i++)
	{
		float attenuation = get_light_attenuation(lights[i]);
		float band_mask = get_band_mask(lights[i].location, 0.99f - attenuation, 0.01f);
		final_color = mix(final_color, lights[i].color, band_mask);	
	}

	// output
	finalColor = vec4(final_color, 1.f);
};