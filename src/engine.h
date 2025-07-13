#ifndef ENGINE_H
#define ENGINE_H

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

// Audio
//#include <pulse/simple.h>
//#include <pulse/error.h>
#include <portaudio.h>
#include <fftw3.h>  

#define PI 3.14159265359

#define MAX_BUFFERS 8
#define MAX_UNIFORMS 32
#define MAX_SHADERS 64
#define MAX_VERTICES 65536
#define MAX_INDICES 65536
#define MAX_PATH_LENGTH 256

typedef bool b8;
typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef char c8;
typedef unsigned char uc8;
typedef size_t sz;

// Structures
typedef struct {
    f32 x, y, z;
} vec3_t;

typedef struct {
    f32 x, y, z, w;
} vec4_t;

typedef struct {
    f32 x, y;
} vec2_t;

typedef struct { float m[16]; } mat4_t;
static mat4_t mat4_identity(void) {
    mat4_t I = { {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    } };
    return I;
}

typedef struct {
    vec3_t position;
    vec3_t normal;
    vec2_t uv;
} vertex_t;

typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    char vertex_path[MAX_PATH_LENGTH];
    char fragment_path[MAX_PATH_LENGTH];
    time_t vertex_mtime;
    time_t fragment_mtime;
    b8 needs_reload;
} shader_t;

typedef struct {
    vertex_t* vertices;
    u32* indices;
    u32 vertex_count;
    u32 index_count;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    shader_t* shader;
    mat4_t matrix;  
} mesh_t;

typedef struct {
    mesh_t* meshes;
    u32 mesh_count;
} model_t;

typedef enum {
    UNIFORM_FLOAT,
    UNIFORM_VEC2,
    UNIFORM_VEC3,
    UNIFORM_VEC4,
    UNIFORM_INT,
    UNIFORM_TEXTURE
} uniform_type_t;

typedef struct {
    char name[64];
    uniform_type_t type;
    union {
        f32 f;
        vec2_t vec2;
        vec3_t vec3;
        vec4_t vec4;
        i32 i;
        GLuint texture;
    } value;
} uniform_t;

typedef struct {
    GLuint framebuffer;
    GLuint texture;
    GLuint depth_buffer;
    u32 width;
    u32 height;
} render_buffer_t;

typedef struct {
    GLFWwindow* window;
    u32 window_width;
    u32 window_height;
    
    // Render buffers
    render_buffer_t buffers[MAX_BUFFERS];
    u32 buffer_count;
    
    // Shaders
    shader_t shaders[MAX_SHADERS];
    u32 shader_count;
    
    // Models
    model_t* models;
    u32 model_count;
    
    // Uniforms
    uniform_t uniforms[MAX_UNIFORMS];
    u32 uniform_count;
    
    // Timing
    f64 time;
    f64 delta_time;
    f64 last_frame_time;
    i32 frame_count;
    
    // Input
    b8 keys[1024];
    f64 mouse_x, mouse_y;
    f64 mouse_dx, mouse_dy;
    b8 mouse_buttons[8];
    
    // Quad for fullscreen rendering
    GLuint quad_vao;
    GLuint quad_vbo;
} engine_t;

// Engine functions
b8 engine_init(engine_t* engine, u32 width, u32 height, const char* title);
void engine_cleanup(engine_t* engine);
b8 engine_should_close(engine_t* engine);
void engine_update(engine_t* engine);
void engine_render_quad(engine_t* engine);
void engine_render(engine_t* engine);
void engine_clear();
void engine_swap_buffers(engine_t* engine);
void engine_poll_events(engine_t* engine);
void engine_check_exit_keys(engine_t* engine, i32* keys, i32 key_count);

// Shader functions
shader_t* shader_load(engine_t* engine, const char* vertex_path, const char* fragment_path);
b8 shader_reload_if_changed(shader_t* shader);
void shader_use(engine_t* engine, shader_t* shader, const b8 update_uniforms);
void shader_cleanup(shader_t* shader);
GLuint shader_get_uniform_location(shader_t* shader, const char* name);

// Mesh functions
void mesh_translate(mesh_t* mesh, const vec3_t* v);
void mesh_rotate(mesh_t* mesh, const vec3_t* v, f32 angle);
void mesh_scale(mesh_t* mesh, const vec3_t* v);

// Model functions
b8 model_load_obj(model_t* model, const char* path, shader_t* shaders);
void model_render(engine_t* engine, model_t* model);
void model_cleanup(model_t* model);
void model_translate(model_t* model, const vec3_t* v);
void model_rotate(model_t* model, const vec3_t* v, f32 angle);
void model_scale(model_t* model, const vec3_t* v);

// Buffer functions
b8 render_buffer_create(render_buffer_t* buffer, u32 width, u32 height);
void render_buffer_bind(render_buffer_t* buffer);
void render_buffer_unbind(void);
void render_buffer_cleanup(render_buffer_t* buffer);

// Uniform functions
void uniform_set_float(engine_t* engine, const char* name, f32 value);
void uniform_set_vec2(engine_t* engine, const char* name, vec2_t value);
void uniform_set_vec3(engine_t* engine, const char* name, vec3_t value);
void uniform_set_vec4(engine_t* engine, const char* name, vec4_t value);
void uniform_set_int(engine_t* engine, const char* name, i32 value);
void uniform_set_texture(engine_t* engine, const char* name, GLuint texture);
void uniform_set_buffer_texture(engine_t* engine, const char* name, render_buffer_t* buffer);
void uniform_apply(engine_t* engine, shader_t* shader); // make sure the shader is in use

// Utility functions
f64 get_time(void);
f64 get_delta_time(const engine_t* engine);
time_t get_file_mtime(const char* path);
char* load_file(const char* path);
void create_fullscreen_quad(GLuint* vao, GLuint* vbo);

// Audio
void audio_init();
void audio_cleanup();
vec3_t audio_get_amplitudes();

// Math (TODO: Change all args to ptrs for performance)
#define PI 3.14159265359
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
f32 vec3_length(vec3_t v);
mat4_t mat4_perspective(float fov_y, float aspect, float near, float far);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_norm(vec3_t v);
vec3_t vec3_cross(vec3_t a, vec3_t b);
mat4_t mat4_look_at(vec3_t eye, vec3_t center, vec3_t up);
mat4_t mat4_mul(const mat4_t A, const mat4_t B);
mat4_t mat4_translate(const vec3_t* v); 
mat4_t mat4_rotate_x(mat4_t m, f32 angle);
mat4_t mat4_rotate_y(mat4_t m, f32 angle);
mat4_t mat4_rotate_z(mat4_t m, f32 angle);
mat4_t mat4_scale(const vec3_t* v);

#endif // ENGINE_H
