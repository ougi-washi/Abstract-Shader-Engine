// Abstract-Shader-Engine - Ougi Washi

#ifndef AS_RENDER_H
#define AS_RENDER_H

#include "as_types.h"
#include <GLFW/glfw3.h>
#include <time.h>
#include <assert.h>
#include <pthread.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

#define PI 3.14159265359

#define MAX_BUFFERS 8
#define MAX_UNIFORMS 32
#define MAX_SHADERS 64
#define MAX_MESHES 64
#define MAX_VERTICES 65536
#define MAX_INDICES 65536
#define MAX_PATH_LENGTH 256

static as_mat4 mat4_identity(void) {
    as_mat4 I = { {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    } };
    return I;
}

typedef struct {
    as_vec3 position;
    as_vec3 normal;
    as_vec2 uv;
} as_vertex;

typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    char vertex_path[MAX_PATH_LENGTH];
    char fragment_path[MAX_PATH_LENGTH];
    time_t vertex_mtime;
    time_t fragment_mtime;
    b8 needs_reload;
} as_shader;

typedef struct {
    as_vertex* vertices;
    u32* indices;
    u32 vertex_count;
    u32 index_count;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    as_shader* shader;
    as_mat4 matrix;  
} as_mesh;

typedef struct {
    as_mesh* meshes;
    u32 as_mesh_count;
} as_model;

typedef enum {
    as_uniform_FLOAT,
    as_uniform_VEC2,
    as_uniform_VEC3,
    as_uniform_VEC4,
    as_uniform_INT,
    as_uniform_TEXTURE
} as_uniform_type;

typedef struct {
    char name[64];
    as_uniform_type type;
    union {
        f32 f;
        as_vec2 vec2;
        as_vec3 vec3;
        as_vec4 vec4;
        i32 i;
        GLuint texture;
    } value;
} as_uniform;

typedef struct {
    GLuint framebuffer;
    GLuint texture;
    GLuint depth_buffer;
    u32 width;
    u32 height;
} as_render_buffer;

typedef struct {
    GLFWwindow* window;
    u32 window_width;
    u32 window_height;
    
    as_render_buffer buffers[MAX_BUFFERS];
    u32 buffer_count;
    
    as_shader shaders[MAX_SHADERS];
    u32 as_shader_count;
    
    as_model* models;
    u32 as_model_count;
    
    as_uniform uniforms[MAX_UNIFORMS];
    u32 as_uniform_count;
    
    f64 time;
    f64 delta_time;
    f64 last_frame_time;
    i32 frame_count;
    
    b8 keys[1024];
    f64 mouse_x, mouse_y;
    f64 mouse_dx, mouse_dy;
    b8 mouse_buttons[8];
    
    GLuint quad_vao;
    GLuint quad_vbo;
} as_engine;

// Engine functions
b8 as_engine_init(as_engine* engine, u32 width, u32 height, const char* title);
void as_engine_cleanup(as_engine* engine);
b8 as_engine_should_close(as_engine* engine);
void as_engine_update(as_engine* engine);
void as_engine_render_quad(as_engine* engine);
void as_engine_render(as_engine* engine);
void as_engine_clear();
void as_engine_swap_buffers(as_engine* engine);
void as_engine_poll_events(as_engine* engine);
void as_engine_check_exit_keys(as_engine* engine, i32* keys, i32 key_count);

// Shader functions
as_shader* as_shader_load(as_engine* engine, const char* vertex_path, const char* fragment_path);
b8 as_shader_reload_if_changed(as_shader* shader);
void as_shader_use(as_engine* engine, as_shader* shader, const b8 update_uniforms);
void as_shader_cleanup(as_shader* shader);
GLuint as_shader_get_uniform_location(as_shader* shader, const char* name);

// Mesh functions
void as_mesh_translate(as_mesh* mesh, const as_vec3* v);
void as_mesh_rotate(as_mesh* mesh, const as_vec3* v);
void as_mesh_scale(as_mesh* mesh, const as_vec3* v);

// Model functions
b8 as_model_load_obj(as_model* model, const char* path, as_shader** shaders, const sz as_shader_count); 
void as_model_render(as_engine* engine, as_model* model);
void as_model_cleanup(as_model* model);
void as_model_translate(as_model* model, const as_vec3* v);
void as_model_rotate(as_model* model, const as_vec3* v);
void as_model_scale(as_model* model, const as_vec3* v);

// Buffer functions
b8 as_render_buffer_create(as_render_buffer* buffer, u32 width, u32 height);
void as_render_buffer_bind(as_render_buffer* buffer);
void as_render_buffer_unbind(void);
void as_render_buffer_cleanup(as_render_buffer* buffer);

// Uniform functions
void as_uniform_set_float(as_engine* engine, const char* name, f32 value);
void as_uniform_set_vec2(as_engine* engine, const char* name, as_vec2 value);
void as_uniform_set_vec3(as_engine* engine, const char* name, as_vec3 value);
void as_uniform_set_vec4(as_engine* engine, const char* name, as_vec4 value);
void as_uniform_set_int(as_engine* engine, const char* name, i32 value);
void as_uniform_set_texture(as_engine* engine, const char* name, GLuint texture);
void as_uniform_set_buffer_texture(as_engine* engine, const char* name, as_render_buffer* buffer);
void as_uniform_apply(as_engine* engine, as_shader* shader); // make sure the shader is in use

// Utility functions
f64 get_time(void);
f64 get_delta_time(const as_engine* engine);
time_t get_file_mtime(const char* path);
char* load_file(const char* path);
void create_fullscreen_quad(GLuint* vao, GLuint* vbo);

// Math (TODO: Change all args to ptrs for performance)
#define PI 3.14159265359
#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))
f32 vec3_length(as_vec3 v);
as_mat4 mat4_perspective(float fov_y, float aspect, float near, float far);
as_vec3 vec3_sub(as_vec3 a, as_vec3 b);
as_vec3 vec3_norm(as_vec3 v);
as_vec3 vec3_cross(as_vec3 a, as_vec3 b);
as_mat4 mat4_look_at(as_vec3 eye, as_vec3 center, as_vec3 up);
as_mat4 mat4_mul(const as_mat4 A, const as_mat4 B);
as_mat4 mat4_translate(const as_vec3* v); 
as_mat4 mat4_rotate_x(as_mat4 m, f32 angle);
as_mat4 mat4_rotate_y(as_mat4 m, f32 angle);
as_mat4 mat4_rotate_z(as_mat4 m, f32 angle);
as_mat4 mat4_scale(const as_vec3* v);

#endif // AS_RENDER_H
