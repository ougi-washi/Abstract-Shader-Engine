#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;

// Model-View-Projection and Model matrices
uniform mat4 u_mvp;
uniform mat4 u_model;

out vec2 v_uv;
out vec3 v_normal;
out vec3 v_frag_pos;

void main() {
    gl_Position = u_mvp * vec4(a_Position, 1.0);

    v_frag_pos  = vec3(u_model * vec4(a_Position, 1.0));
    v_normal   = mat3(transpose(inverse(u_model))) * a_Normal;

    v_uv       = a_UV;
}
