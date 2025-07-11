#version 330 core

//uniform float time;
uniform vec2 resolution;
uniform vec2 mouse;
uniform int frame;
uniform float amplitude;
uniform float frequency;
in vec2 TexCoord;
out vec4 FragColor;

void main() {
    vec2 uv = TexCoord;
    vec2 center = vec2(0.5);
    float dist = distance(uv, center) * (amplitude) - frame * 0.01;
    dist = fract(dist);
    vec3 color = vec3(dist);
    FragColor = vec4(color, 1.0);
}
