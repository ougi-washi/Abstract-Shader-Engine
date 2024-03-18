#version 450

layout(location = 0) out vec2 out_uv;

void main()
{
    //gl_Position = vec4(ubo.position, 0.0, 1.0);
    //out_tex_coord = in_tex_coord;

    out_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4((out_uv * 2.0f + -1.0f), 0.0f, 1.0f);
}
