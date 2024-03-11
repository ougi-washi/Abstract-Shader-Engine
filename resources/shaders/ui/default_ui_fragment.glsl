#version 450

layout(binding = 0) uniform uniform_buffer_screen_object 
{
    mat4 data;
} ubo; 
layout(binding = 1) uniform sampler2D tex_sampler;
layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 out_color;

void main()
{
    out_color = texture(tex_sampler, uv);
    vec2 center_uv = uv - vec2(.5);
    float sphere_mask = length(center_uv) - .3f;
    sphere_mask = smoothstep(0.0, 1., sphere_mask);
    out_color = vec4(uv.y * out_color.x, out_color.y, out_color.z, sphere_mask);
}
