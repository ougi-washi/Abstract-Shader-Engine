#version 330 core

//uniform float time;
uniform vec2 resolution;
uniform vec2 mouse;
uniform int frame;
uniform sampler2D buffer1;
in vec2 TexCoord;
out vec4 FragColor;

void main() {
    FragColor = texture(buffer1, TexCoord);
}
