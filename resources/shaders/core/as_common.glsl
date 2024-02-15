// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

layout(binding = 0) uniform uniform_buffer_object 
{
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo; 

layout(push_constant) uniform push_constant_buffer
{
    mat4 data;
    // camera_position  X[0][0] Y[0][1] Z[0][2]
	// camera_direction X[1][0] Y[1][1] Z[1][2]
	// current_time		[2][0]
	// object_index		[2][1]
	// mouse_data		X[3][0] Y[3][1]
} ps;

vec3 get_camera_pos() { return vec3(ps.data[0][0], ps.data[0][1], ps.data[0][2]); }
vec3 get_camera_dir() { return vec3(ps.data[1][0], ps.data[1][1], ps.data[1][2]); }
float get_current_time() { return ps.data[2][0]; }
int get_object_index() { return int(ps.data[2][1]); }
vec2 get_mouse_pos() { return vec2(ps.data[3][0], ps.data[3][1]); }

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
