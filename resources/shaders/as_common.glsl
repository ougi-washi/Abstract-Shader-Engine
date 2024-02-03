// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

layout(push_constant) uniform push_constant_buffer
{
	mat4 object_transform;
	vec3 camera_location;
	vec3 camera_direction;
	vec4 mouse_data;
	float current_time;
} ps;
