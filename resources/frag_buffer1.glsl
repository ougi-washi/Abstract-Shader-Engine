#version 330 core

uniform float time;
//uniform vec2 resolution;
//uniform vec2 mouse;
uniform float delta_time;
uniform int frame;
uniform vec3 amps;
uniform float frequency;
in vec2 TexCoord;
out vec4 FragColor;

void main() {
    vec2 uv = TexCoord;
    vec2 center = vec2(0.5);
    vec3 adjusted_amps = amps;
    adjusted_amps.x = amps.x * 5.;
    float dist = distance(uv, center) * 40 * (adjusted_amps.x  + adjusted_amps.y * 0.5) - time ;
    dist = fract(dist);
    vec3 color = vec3(dist);
    color *= amps + .1 ;
    FragColor = vec4(color, 1.0);
}
