#version 330 core

in vec3 vertex_color;
in vec2 tex_coord;
out vec4 out_frag_color;  

uniform sampler2D uniform_texture;
uniform sampler2D uniform_texture1;


void main()
{
//    out_frag_color = mix(texture(uniform_texture, tex_coord), texture(uniform_texture1, tex_coord), .5f);
//	out_frag_color = texture(uniform_texture, tex_coord);
	out_frag_color = vec4(1.f);
}