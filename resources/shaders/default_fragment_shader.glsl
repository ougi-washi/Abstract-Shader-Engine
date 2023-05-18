#version 330

#define MAX_LIGHTS_PER_WORLD 64
struct light
{
	vec3 location;
	float intensity;
	float attenuation;
	vec3 color;
};

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

// Output fragment color
out vec4 finalColor;

uniform int lights_count;
uniform light lights[MAX_LIGHTS_PER_WORLD];

float get_light_attenuation(const light in_light);
float get_band_mask(const vec3 light_location, const float offset, const float range);

void main()
{
	vec3 final_color = vec3(.1);

	// first ambient pass
	for(int i = 0 ; i < lights_count ; i++)
	{
		float distance_to_light = length(lights[i].location - fragPosition);
		float attenuation = get_light_attenuation(lights[i]);
		final_color += (dot(normalize(fragNormal), normalize(lights[i].location)) * lights[i].intensity * lights[i].color * attenuation);
	}

	// halation
	for(int i = 0 ; i < lights_count ; i++)
	{
		float attenuation = get_light_attenuation(lights[i]);
		float band_mask = get_band_mask(lights[i].location, 0.99 - attenuation, 0.01);
		final_color = mix(final_color, lights[i].color, band_mask);	
	}

    finalColor = vec4(vec3(final_color), 1.f);
}

float get_light_attenuation(const light in_light)
{
	return (1.f / distance(in_light.location, fragPosition)) * in_light.attenuation;
}

float get_band_mask(const vec3 light_location, const float offset, const float range)
{
	return smoothstep(offset, offset + range, dot(normalize(fragNormal), normalize(light_location)));
};