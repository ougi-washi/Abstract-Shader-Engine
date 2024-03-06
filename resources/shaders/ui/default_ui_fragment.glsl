#version 450

layout(location = 0) in vec2 uv;

//layout(set = 0, binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) out vec4 out_color;

void main()
{
    //out_color = texture(tex_sampler, out_uv);
    vec2 center_uv = uv - vec2(.5);
    float sphere_mask = length(center_uv) - .1f;
    sphere_mask = smoothstep(0.1, 1., sphere_mask);
    out_color = vec4(uv.y, 0.f, 1., sphere_mask);
}
