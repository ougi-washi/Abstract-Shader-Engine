#version 330 core

#define MAX_LIGHTS_PER_WORLD 64
#define NEAR_PLANE 0.1
#define FAR_PLANE 100.0

struct light
{
	vec3 location;
	float intensity;
	float attenuation;
	vec3 color;
};
in vec3 position;
in vec3 vertex_color;
in vec2 tex_coord;
in vec3 normal;
out vec4 out_frag_color;  

uniform sampler2D uniform_texture;
uniform sampler2D uniform_texture1;
uniform int light_count;
uniform light lights[MAX_LIGHTS_PER_WORLD];

float linearize_depth(const float depth);
float get_depth();
float get_band_mask(const vec3 light_location, const float offset, const float range);

void main()
{
	vec3 final_color = vec3(.0);
	
	for(int i = 0 ; i < light_count ; i++)
	{
		float distance_to_light = length(lights[i].location - position);
		float attenuation = 1/(1 + 0.09 * distance_to_light + 0.032 * pow(distance_to_light , 2));
		final_color += (dot(normalize(normal), normalize(lights[i].location)) * lights[i].intensity * lights[i].color );
	}

	out_frag_color = vec4(vec3(get_depth()), 1.0);
}

float get_band_mask(const vec3 light_location, const float offset, const float range)
{
	return smoothstep(dot(normalize(normal), normalize(light_location)) , .8, -.9);
};

float get_depth()
{
	return linearize_depth(gl_FragCoord.z) / FAR_PLANE;
}

float linearize_depth(const float depth)
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}