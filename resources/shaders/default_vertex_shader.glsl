#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec2 in_tex_coord;
layout (location = 3) in vec3 in_tangent;
layout (location = 4) in vec3 in_bitangent;
layout (location = 5) in float[] in_ids;
layout (location = 6) in int[] in_weights;

out vec3 position;
out vec2 tex_coord;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(in_pos, 1.0);
    position = in_pos;
    tex_coord = in_tex_coord;
    normal = in_normal;
}