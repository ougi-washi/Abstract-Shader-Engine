#version 330 core

uniform float time;
uniform vec2 resolution;
uniform vec2 mouse;
in vec2 TexCoord;
out vec4 FragColor;

void main() {
    vec2 uv = TexCoord;
    vec2 center = vec2(0.5);
    float dist = distance(uv, center);
    float wave = sin(dist * 10.0 - time * 3.0) * 0.5 + 0.5;
    vec3 color = vec3(wave, sin(time) * 0.5 + 0.5, cos(time) * 0.5 + 0.5);
    FragColor = vec4(color, 1.0);
}
