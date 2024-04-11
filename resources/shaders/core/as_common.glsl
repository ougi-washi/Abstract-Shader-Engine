// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#define AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE 128
layout(binding = 0) uniform uniform_buffer_object 
{
    mat4 model;
    mat4 view;
    mat4 proj;
	mat4 scene_info;
    mat4 objects_transforms[AS_MAX_GPU_OBJECT_TRANSFORMS_SIZE];
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

vec3 get_position(mat4 transform) { return vec3(transform[3][0], transform[3][1], transform[3][2]); }
vec3 get_scale(mat4 transform) { return vec3(transform[0][0], transform[1][1], transform[2][2]); }
vec3 get_camera_pos() { return vec3(ps.data[0][0], ps.data[0][1], ps.data[0][2]); }
vec3 get_camera_dir() { return vec3(ps.data[1][0], ps.data[1][1], ps.data[1][2]); }
float get_current_time() { return ps.data[2][0]; }
int get_object_index() { return int(ps.data[2][1]); }
vec2 get_mouse_pos() { return vec2(ps.data[3][0], ps.data[3][1]); }
mat4 get_object_transform(int index) { return ubo.objects_transforms[index]; }
vec3 get_object_position(int index) { return get_position(get_object_transform(index)); }
vec3 get_object_scale(int index) { return get_scale(get_object_transform(index)); }
mat4 get_current_object_transform() { return ubo.objects_transforms[get_object_index()]; }
vec3 get_current_object_position() { return get_object_position(get_object_index()); }
int get_object_count() { return int(ubo.scene_info[0][0]); }

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
