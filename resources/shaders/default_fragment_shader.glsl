#version 330 core

in vec3 vertex_color;
in vec2 tex_coord;
out vec4 out_frag_color;  
uniform sampler2D uniform_texture;

void main()
{
    out_frag_color = texture(uniform_texture, tex_coord) * vec4(vertex_color, 1.0);
}