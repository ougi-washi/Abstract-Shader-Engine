// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

layout(push_constant) uniform push_constant_buffer
{
	mat4 object_transform;
	vec3 camera_location;
	vec3 camera_direction;
	vec4 mouse_data;
	float current_time;
} ps;

mat4 look_at(vec3 eye, vec3 center, vec3 up) 
{
    vec3 f = normalize(center - eye);
    vec3 r = normalize(cross(up, f));
    vec3 u = cross(f, r);

    return mat4(
        vec4(r, 0.0),
        vec4(u, 0.0),
        vec4(-f, 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    ) * mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(-eye, 1.0)
    );
}