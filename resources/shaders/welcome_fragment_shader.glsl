#include "resources/shaders/common/fragment_shader_common.glsl"

uniform sampler2D noise;

float get_sphere_mask(
	const vec2 position, const vec2 uv, const float max_res,
	const float size, const float smoothness)
{
	return smoothstep(
		     size + smoothness,
			size,
			distance(uv * 1.01, position * 1.01));
}

vec3 get_color(float in_time)
{
    float max_res = 2.;
	vec2 uv = fragTexCoord;
	vec2 center = resolution / 2.;
	vec3 color = vec3(0.0);

	float seed = 20.;
	float count = 20.;
	float speed = .25;
	float momentum = in_time * speed;
	float momentum_scale = 5.;
	float noise_scale = 1.5;
	if (distance(uv, center) < .1)
	{
		noise_scale += 10.;
	}

	float noise_dir =
		abs(sin(in_time * .01 - distance(uv, center)* 15.));
	vec2 light_dir = vec2(sin(momentum), cos(momentum));
	float light_attenuation = dot(light_dir, uv) + .1;
	for (float i = 0.; i < count; ++i)
	{
		if (mod(i, 2.) > 1.)
		{
			momentum /= momentum_scale;
		}
		vec2 random_offset = texture2D(
			noise,
			vec2(uv.x + seed + noise_dir * sin(momentum * .01 * i) * .2,
				uv.y + seed + noise_dir * cos(i) * .1)/noise_scale).xy;

		if (mod(random_offset.x * 100., 2.) > 1.)
		{
			random_offset.x *= -1.;
		}
		if (mod(random_offset.y * 10., 2.) > 1.)
		{
			random_offset.y *= -1.;
		}
		random_offset.x = random_offset.x  * sin( .1*i* momentum) * sin(i + seed) ;
		random_offset.y = random_offset.y * cos(.1*i *momentum) * sin(i+ seed);

		color +=
			vec3(get_sphere_mask(
				vec2(.5 + random_offset),
				uv, 2.,
				.01, .05));
		color *=
			vec3(light_attenuation * 1.3,
				.3,
				(1.-light_attenuation) * 1.) ;
	}
    return color;
}

void main() {

	// vec4 previous_frame =
	// 	texture2D(backbuffer, gl_FragCoord.xy / resolution);
	// vec3 color = get_color(time);
    vec3 color_prev = get_color(time - .3) * .2;
    vec3 color_now = get_color(time);
     
	// color = min(color, vec3(.8));
	finalColor = vec4(vec3(color_now + color_prev),1.0);
}