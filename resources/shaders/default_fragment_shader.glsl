#version 330 core

in vec3 vertex_color;
in vec2 tex_coord;
in vec3 normal;
out vec4 out_frag_color;  

uniform sampler2D uniform_texture;
uniform sampler2D uniform_texture1;

void main()
{
	out_frag_color = vec4(tex_coord.x, tex_coord.y, 0.0, 1.0);	
}