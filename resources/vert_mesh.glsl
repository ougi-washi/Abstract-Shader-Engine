#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;

// Model-View-Projection and Model matrices
uniform mat4 u_MVP;
uniform mat4 u_Model;

out vec2 vUV;
out vec3 vNormal;
out vec3 vFragPos;

void main() {
    gl_Position = u_MVP * vec4(a_Position, 1.0);

    vFragPos  = vec3(u_Model * vec4(a_Position, 1.0));
    vNormal   = mat3(transpose(inverse(u_Model))) * a_Normal;

    vUV       = a_UV;
}
