#version 330 core

#define MAX_LIGHTS_PER_WORLD 64
struct light
{
	vec3 location;
	float intensity;
	float attenuation;
	vec3 color;
};

in vec3 vertex_color;
in vec2 tex_coord;
in vec3 normal;
out vec4 out_frag_color;  

uniform sampler2D uniform_texture;
uniform sampler2D uniform_texture1;
uniform int light_count;
uniform light lights[MAX_LIGHTS_PER_WORLD];

float get_band_mask(const vec3 light_location, const float offset, const float range);

void main()
{
	vec3 final_color = vec3(.1);
	
	for(int i = 0 ; i < light_count ; i++)
	{
		final_color += (dot(normalize(normal), normalize(lights[i].location)) * lights[i].intensity * lights[i].color );
		vec3 band_color = (lights[i].color * lights[i].intensity);

	}
	float band_mask = get_band_mask(lights[0].location, 0.0, 0.);
	final_color = mix(final_color, lights[0].color, band_mask);	

	band_mask = get_band_mask(lights[1].location, 0.0, 0.);
	final_color = mix(final_color, lights[1].color, band_mask);	

	out_frag_color = vec4(final_color, 1.0);
}

float get_band_mask(const vec3 light_location, const float offset, const float range)
{
	return smoothstep(dot(normalize(normal), normalize(light_location)) , .85, -.9);
};