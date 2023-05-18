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

float get_band_mask(const vec3 light_location, const float offset, const float range);

void main()
{
	vec3 final_color = vec3(.1);

	for(int i = 0 ; i < lights_count ; i++)
	{
		float distance_to_light = length(lights[i].location - fragPosition);
		float attenuation = 1/(1 + 0.09 * distance_to_light + 0.032 * pow(distance_to_light , 2));
		final_color += (dot(normalize(fragNormal), normalize(lights[i].location)) * lights[i].intensity * lights[i].color );
		//final_color *= vec3(1/(.002 * pow(distance_to_light, 2)));
	}

	for(int i = 0 ; i < lights_count ; i++)
	{
		float band_mask = get_band_mask(lights[i].location, 0.0, 0.);
		final_color = mix(final_color, lights[i].color, band_mask);	
	}

    finalColor = vec4(vec3(final_color), 1.f);
}

float get_band_mask(const vec3 light_location, const float offset, const float range)
{
	return smoothstep(dot(normalize(fragNormal), normalize(light_location)) , .8, -.9);
};