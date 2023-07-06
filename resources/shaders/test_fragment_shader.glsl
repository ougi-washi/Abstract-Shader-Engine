#include "resources/shaders/common/fragment_shader_common.glsl"

//https://iquilezles.org/articles/palettes/
vec3 palette(float t) 
{
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.263,0.416,0.557);

    return a + b*cos( 6.28318*(c*t+d) );
}

void main()
{
	vec2 uv = fragTexCoord - .5;
	vec2 uv0 = uv;
	vec3 final_color = vec3(0.0);

    for (float i = 0.0; i < 4.0; i++) {
        uv = fract(uv * 1.5) - 0.5;

        float d = length(uv) * exp(-length(uv0));

        vec3 col = palette(length(uv0) + i*.4 + time*.4);

        d = sin(d*8. + time)/8.;
        d = abs(d);

        d = pow(0.01 / d, 1.2);
		
		
		for(int i = 0 ; i < lights_count ; i++)
		{ 
        	// Ambient lighting
			final_color += (calculate_light(lights[i]) + .1) * col * d;
		}
        // final_color += col * d;
    }

	// output
	finalColor = vec4(final_color, 1.f);
};