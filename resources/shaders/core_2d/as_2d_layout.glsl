// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

//#define AS_MAX_GPU_SCREEN_OBJECT_CUSTOM_INFO_SIZE 16 // has to match the cpu side
#define AS_MAX_GPU_SCREEN_OBJECT_CUSTOM_DATA_SIZE 256 // has to match the cpu side
#define AS_SCREEN_OBJECT_DATA_OFFSET 4

layout(binding = 0) uniform uniform_buffer_screen_object 
{
	uint custom_data[AS_MAX_GPU_SCREEN_OBJECT_CUSTOM_DATA_SIZE];
} ubo; 

layout(push_constant) uniform push_constant_buffer
{
    mat4 data;
	// position	x[0][0] y[0][1]
	// rotation	x[0][2] y[0][3]
	// extent	x[1][0] y[1][1]
	// [2][...]	empty
	// [3][...]	empty
} ps;

vec2 get_2d_position() {return vec2(ps.data[0][0], ps.data[0][1]); }
