#ifndef ENGINE_H
#define ENGINE_H

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

// Constants
#define MAX_BUFFERS 8
#define MAX_UNIFORMS 32
#define MAX_VERTICES 65536
#define MAX_INDICES 65536
#define MAX_SHADER_SIZE 16384
#define MAX_PATH_LENGTH 256

// Structures
typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    float x, y, z, w;
} vec4_t;

typedef struct {
    float x, y;
} vec2_t;

typedef struct {
    vec3_t position;
    vec3_t normal;
    vec2_t uv;
} vertex_t;

typedef struct {
    vertex_t* vertices;
    uint32_t* indices;
    uint32_t vertex_count;
    uint32_t index_count;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
} mesh_t;

typedef struct {
    mesh_t* meshes;
    uint32_t mesh_count;
} model_t;

typedef struct {
    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    char vertex_path[MAX_PATH_LENGTH];
    char fragment_path[MAX_PATH_LENGTH];
    time_t vertex_mtime;
    time_t fragment_mtime;
    bool needs_reload;
} shader_t;

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
        float f;
        vec2_t vec2;
        vec3_t vec3;
        vec4_t vec4;
        int i;
        GLuint texture;
    } value;
} uniform_t;

typedef struct {
    GLuint framebuffer;
    GLuint texture;
    GLuint depth_buffer;
    uint32_t width;
    uint32_t height;
} render_buffer_t;

typedef struct {
    GLFWwindow* window;
    uint32_t window_width;
    uint32_t window_height;
    
    // Render buffers
    render_buffer_t buffers[MAX_BUFFERS];
    uint32_t buffer_count;
    
    // Shaders
    shader_t* shaders;
    uint32_t shader_count;
    
    // Models
    model_t* models;
    uint32_t model_count;
    
    // Uniforms
    uniform_t uniforms[MAX_UNIFORMS];
    uint32_t uniform_count;
    
    // Timing
    double time;
    double delta_time;
    double last_frame_time;
    uint32_t frame_count;
    
    // Input
    bool keys[1024];
    double mouse_x, mouse_y;
    double mouse_dx, mouse_dy;
    bool mouse_buttons[8];
    
    // Quad for fullscreen rendering
    GLuint quad_vao;
    GLuint quad_vbo;
    
    // Default shader
    shader_t default_shader;
} engine_t;

// Engine functions
bool engine_init(engine_t* engine, uint32_t width, uint32_t height, const char* title);
void engine_cleanup(engine_t* engine);
bool engine_should_close(engine_t* engine);
void engine_update(engine_t* engine);
void engine_render(engine_t* engine);
void engine_clear();
void engine_swap_buffers(engine_t* engine);
void engine_poll_events(engine_t* engine);

// Shader functions
bool shader_load(shader_t* shader, const char* vertex_path, const char* fragment_path);
bool shader_reload_if_changed(shader_t* shader);
void shader_use(shader_t* shader);
void shader_cleanup(shader_t* shader);
GLuint shader_get_uniform_location(shader_t* shader, const char* name);

// Model functions
bool model_load_obj(model_t* model, const char* path);
void model_render(model_t* model);
void model_cleanup(model_t* model);

// Buffer functions
bool render_buffer_create(render_buffer_t* buffer, uint32_t width, uint32_t height);
void render_buffer_bind(render_buffer_t* buffer);
void render_buffer_unbind(void);
void render_buffer_cleanup(render_buffer_t* buffer);

// Uniform functions
void uniform_set_float(engine_t* engine, const char* name, float value);
void uniform_set_vec2(engine_t* engine, const char* name, vec2_t value);
void uniform_set_vec3(engine_t* engine, const char* name, vec3_t value);
void uniform_set_vec4(engine_t* engine, const char* name, vec4_t value);
void uniform_set_int(engine_t* engine, const char* name, int value);
void uniform_set_texture(engine_t* engine, const char* name, GLuint texture);
void uniform_apply_all(engine_t* engine, shader_t* shader);

// Utility functions
double get_time(void);
time_t get_file_mtime(const char* path);
char* load_file(const char* path);
void create_fullscreen_quad(GLuint* vao, GLuint* vbo);

// Math utilities
vec2_t vec2_create(float x, float y);
vec3_t vec3_create(float x, float y, float z);
vec4_t vec4_create(float x, float y, float z, float w);
float vec3_length(vec3_t v);
vec3_t vec3_normalize(vec3_t v);
vec3_t vec3_cross(vec3_t a, vec3_t b);

#endif // ENGINE_H
