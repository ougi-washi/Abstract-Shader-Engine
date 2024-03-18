// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#define AS_MAX_GPU_SCREEN_OBJECT_CUSTOM_DATA_SIZE 512 // has to match the cpu side

layout(binding = 0) uniform uniform_buffer_screen_object 
{
	mat4 data;
	// position	x[0][0] y[0][1]
	// rotation	x[0][2] y[0][3]
	// extent	x[1][0] y[1][1]
	// [2][...]	empty
	// [3][...]	empty
	uint custom_data[AS_MAX_GPU_SCREEN_OBJECT_CUSTOM_DATA_SIZE];
} ubo; 
