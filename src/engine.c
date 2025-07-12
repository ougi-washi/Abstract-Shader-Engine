#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pulse/simple.h>
#include <pulse/error.h>

// Fullscreen quad vertex shader
static const char* quad_vertex_shader = 
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"   TexCoord = aTexCoord;\n"
"   gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}\n";

static const char* resources_path = "./resources/";

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

// Forward declarations
static void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods);
static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
static void mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods);
static void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height);
static GLuint compile_shader(const char* source, GLenum type);
static GLuint create_shader_program(const char* vertex_source, const char* fragment_source);

// Engine functions
b8 engine_init(engine_t* engine, uint32_t width, uint32_t height, const char* title) {
    memset(engine, 0, sizeof(engine_t));
    
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }
    
    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    engine->window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!engine->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }
    
    engine->window_width = width;
    engine->window_height = height;
    
    glfwMakeContextCurrent(engine->window);
    glfwSetWindowUserPointer(engine->window, engine);
    
    // Set callbacks
    glfwSetKeyCallback(engine->window, key_callback);
    glfwSetCursorPosCallback(engine->window, mouse_callback);
    glfwSetMouseButtonCallback(engine->window, mouse_button_callback);
    glfwSetFramebufferSizeCallback(engine->window, framebuffer_size_callback);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Create fullscreen quad
    create_fullscreen_quad(&engine->quad_vao, &engine->quad_vbo);

    // Initialize timing
    engine->last_frame_time = get_time();
    
    return true;
}

void engine_cleanup(engine_t* engine) {
    // Cleanup shaders
    for (uint32_t i = 0; i < engine->shader_count; i++) {
        shader_cleanup(&engine->shaders[i]);
    }
   
    // Cleanup models
    for (uint32_t i = 0; i < engine->model_count; i++) {
        model_cleanup(&engine->models[i]);
    }
    free(engine->models);
    
    // Cleanup buffers
    for (uint32_t i = 0; i < engine->buffer_count; i++) {
        render_buffer_cleanup(&engine->buffers[i]);
    }
    
    // Cleanup quad
    glDeleteVertexArrays(1, &engine->quad_vao);
    glDeleteBuffers(1, &engine->quad_vbo);
    
    // Cleanup GLFW
    glfwDestroyWindow(engine->window);
    glfwTerminate();
}

b8 engine_should_close(engine_t* engine) {
    return glfwWindowShouldClose(engine->window);
}

void engine_update(engine_t* engine) {
    double current_time = get_time();
    engine->delta_time = current_time - engine->last_frame_time;
    engine->last_frame_time = current_time;
    engine->time = current_time;
    engine->frame_count++;
    
    // Check for shader reloads
    for (uint32_t i = 0; i < engine->shader_count; i++) {
        shader_reload_if_changed(&engine->shaders[i]);
    }
    
    // Update built-in uniforms
    uniform_set_float(engine, "time", engine->time);
    uniform_set_float(engine, "delta_time", engine->delta_time);
    uniform_set_int(engine, "frame", engine->frame_count);
    //uniform_set_vec2(engine, "resolution", (vec2_t){engine->window_width, engine->window_height});
    //uniform_set_vec2(engine, "mouse", (vec2_t){engine->mouse_x, engine->mouse_y});
}

void engine_render_quad(engine_t* engine) {
    glBindVertexArray(engine->quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void engine_render(engine_t* engine) {
    glViewport(0, 0, engine->window_width, engine->window_height);
    engine_clear(); 
    engine_render_quad(engine);
}

void engine_clear(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void engine_swap_buffers(engine_t* engine) {
    glfwSwapBuffers(engine->window);
}

void engine_poll_events(engine_t* engine) {
    glfwPollEvents();
}

void engine_check_exit_keys(engine_t* engine, i32* keys, i32 key_count) {
    if (key_count == 0) {
        return;
    }
    for (i32 i = 0; i < key_count; i++) {
        if (!engine->keys[keys[i]]) {
            return;
        }
    }
    glfwSetWindowShouldClose(engine->window, GLFW_TRUE);
}

// Shader functions

char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

b8 shader_load_internal(shader_t* shader, const char* vertex_path, const char* fragment_path) {
   
    assert(shader && vertex_path != NULL && fragment_path != NULL);

    shader_cleanup(shader);
    // add to path
    char* new_vertex_path = NULL;
    char* new_fragment_path = NULL;
    
    if (strlen(vertex_path) > 0) {
        new_vertex_path = malloc(strlen(resources_path) + strlen(vertex_path) + 1);
        strcpy(new_vertex_path, resources_path);
        strcat(new_vertex_path, vertex_path);
    }
    
    if (strlen(fragment_path) > 0) {
        new_fragment_path = malloc(strlen(resources_path) + strlen(fragment_path) + 1);
        strcpy(new_fragment_path, resources_path);
        strcat(new_fragment_path, fragment_path);
    }

    strncpy(shader->vertex_path, new_vertex_path, MAX_PATH_LENGTH - 1);
    strncpy(shader->fragment_path, new_fragment_path, MAX_PATH_LENGTH - 1);

    char* vertex_source = load_file(new_vertex_path);
    char* fragment_source = load_file(new_fragment_path);
    
    if (!vertex_source || !fragment_source) {
        free(vertex_source);
        free(fragment_source);
        return false;
    }
    
    shader->program = create_shader_program(vertex_source, fragment_source);
    
    free(vertex_source);
    free(fragment_source);
    
    if (!shader->program) {
        return false;
    }
    
    shader->vertex_mtime = get_file_mtime(new_vertex_path);
    shader->fragment_mtime = get_file_mtime(new_fragment_path);
    printf("Shader loaded: %s, %s\n", new_vertex_path, new_fragment_path);
    return true;
}

shader_t* shader_load(engine_t* engine, const char* vertex_path, const char* fragment_path) {
    engine->shader_count++;
    shader_t* new_shader = &engine->shaders[engine->shader_count - 1];
    if (shader_load_internal(new_shader, vertex_path, fragment_path)) {
        return new_shader;
    }
    return NULL;
}

b8 shader_reload_if_changed(shader_t* shader) {
    if (strlen(shader->vertex_path) == 0 || strlen(shader->fragment_path) == 0) {
        return false;
    }
    
    time_t vertex_mtime = get_file_mtime(shader->vertex_path);
    time_t fragment_mtime = get_file_mtime(shader->fragment_path);
    
    if (vertex_mtime != shader->vertex_mtime || fragment_mtime != shader->fragment_mtime) {
        printf("Reloading shader: %s, %s\n", shader->vertex_path, shader->fragment_path);
        return shader_load(shader, shader->vertex_path, shader->fragment_path);
    }
    
    return false;
}

void shader_use(engine_t* engine, shader_t* shader, const b8 update_uniforms) {
    glUseProgram(shader->program);
    if (update_uniforms) {
        uniform_apply(engine, shader);
    }
}

void shader_cleanup(shader_t* shader) {
    if (shader->program) {
        glDeleteProgram(shader->program);
        shader->program = 0;
    }
}

GLuint shader_get_uniform_location(shader_t* shader, const char* name) {
    return glGetUniformLocation(shader->program, name);
}

// Model functions
b8 model_load_obj(model_t* model, const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to open OBJ file: %s\n", path);
        return false;
    }
    
    // Arrays for temporary storage
    vec3_t* temp_vertices = malloc(MAX_VERTICES * sizeof(vec3_t));
    vec3_t* temp_normals = malloc(MAX_VERTICES * sizeof(vec3_t));
    vec2_t* temp_uvs = malloc(MAX_VERTICES * sizeof(vec2_t));
    
    uint32_t vertex_count = 0;
    uint32_t normal_count = 0;
    uint32_t uv_count = 0;
    
    // Final vertex data
    vertex_t* vertices = malloc(MAX_VERTICES * sizeof(vertex_t));
    uint32_t* indices = malloc(MAX_INDICES * sizeof(uint32_t));
    uint32_t final_vertex_count = 0;
    uint32_t index_count = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            // Vertex position
            sscanf(line, "v %f %f %f", &temp_vertices[vertex_count].x, 
                   &temp_vertices[vertex_count].y, &temp_vertices[vertex_count].z);
            vertex_count++;
        } else if (strncmp(line, "vn ", 3) == 0) {
            // Vertex normal
            sscanf(line, "vn %f %f %f", &temp_normals[normal_count].x,
                   &temp_normals[normal_count].y, &temp_normals[normal_count].z);
            normal_count++;
        } else if (strncmp(line, "vt ", 3) == 0) {
            // Vertex texture coordinate
            sscanf(line, "vt %f %f", &temp_uvs[uv_count].x, &temp_uvs[uv_count].y);
            uv_count++;
        } else if (strncmp(line, "f ", 2) == 0) {
            // Face
            uint32_t v1, v2, v3, n1, n2, n3, t1, t2, t3;
            i32 matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                                &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
            
            if (matches == 9) {
                // Create vertices for this face
                for (i32 i = 0; i < 3; i++) {
                    uint32_t vi = (i == 0) ? v1 - 1 : (i == 1) ? v2 - 1 : v3 - 1;
                    uint32_t ni = (i == 0) ? n1 - 1 : (i == 1) ? n2 - 1 : n3 - 1;
                    uint32_t ti = (i == 0) ? t1 - 1 : (i == 1) ? t2 - 1 : t3 - 1;
                    
                    vertices[final_vertex_count].position = temp_vertices[vi];
                    vertices[final_vertex_count].normal = temp_normals[ni];
                    vertices[final_vertex_count].uv = temp_uvs[ti];
                    
                    indices[index_count] = final_vertex_count;
                    final_vertex_count++;
                    index_count++;
                }
            }
        }
    }
    
    fclose(file);
    
    // Create mesh
    model->mesh_count = 1;
    model->meshes = malloc(sizeof(mesh_t));
    mesh_t* mesh = &model->meshes[0];
    
    mesh->vertices = malloc(final_vertex_count * sizeof(vertex_t));
    mesh->indices = malloc(index_count * sizeof(uint32_t));
    memcpy(mesh->vertices, vertices, final_vertex_count * sizeof(vertex_t));
    memcpy(mesh->indices, indices, index_count * sizeof(uint32_t));
    mesh->vertex_count = final_vertex_count;
    mesh->index_count = index_count;
    
    // Create OpenGL objects
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, final_vertex_count * sizeof(vertex_t), mesh->vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(uint32_t), mesh->indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, normal));
    glEnableVertexAttribArray(1);
    
    // UV attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), (void*)offsetof(vertex_t, uv));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    // Cleanup temporary arrays
    free(temp_vertices);
    free(temp_normals);
    free(temp_uvs);
    free(vertices);
    free(indices);
    
    return true;
}

void model_render(model_t* model) {
    for (uint32_t i = 0; i < model->mesh_count; i++) {
        mesh_t* mesh = &model->meshes[i];
        glBindVertexArray(mesh->vao);
        glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

void model_cleanup(model_t* model) {
    for (uint32_t i = 0; i < model->mesh_count; i++) {
        mesh_t* mesh = &model->meshes[i];
        glDeleteVertexArrays(1, &mesh->vao);
        glDeleteBuffers(1, &mesh->vbo);
        glDeleteBuffers(1, &mesh->ebo);
        free(mesh->vertices);
        free(mesh->indices);
    }
    free(model->meshes);
    model->mesh_count = 0;
}

// Buffer functions
b8 render_buffer_create(render_buffer_t* buffer, uint32_t width, uint32_t height) {
    buffer->width = width;
    buffer->height = height;
    
    // Create framebuffer
    glGenFramebuffers(1, &buffer->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffer);
    
    // Create color texture
    glGenTextures(1, &buffer->texture);
    glBindTexture(GL_TEXTURE_2D, buffer->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->texture, 0);
    
    // Create depth buffer
    glGenRenderbuffers(1, &buffer->depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_buffer);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr, "Framebuffer not complete!\n");
        render_buffer_cleanup(buffer);
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void render_buffer_bind(render_buffer_t* buffer) {
    glBindFramebuffer(GL_FRAMEBUFFER, buffer->framebuffer);
    glViewport(0, 0, buffer->width, buffer->height);
}

void render_buffer_unbind(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_buffer_cleanup(render_buffer_t* buffer) {
    if (buffer->framebuffer) {
        glDeleteFramebuffers(1, &buffer->framebuffer);
        buffer->framebuffer = 0;
    }
    if (buffer->texture) {
        glDeleteTextures(1, &buffer->texture);
        buffer->texture = 0;
    }
    if (buffer->depth_buffer) {
        glDeleteRenderbuffers(1, &buffer->depth_buffer);
        buffer->depth_buffer = 0;
    }
}

// Uniform functions
void uniform_set_float(engine_t* engine, const char* name, f32 value) {
    for (uint32_t i = 0; i < engine->uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = UNIFORM_FLOAT;
            engine->uniforms[i].value.f = value;
            return;
        }
    }
    
    if (engine->uniform_count < MAX_UNIFORMS) {
        uniform_t* uniform = &engine->uniforms[engine->uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = UNIFORM_FLOAT;
        uniform->value.f = value;
        engine->uniform_count++;
    }
}

void uniform_set_vec2(engine_t* engine, const char* name, vec2_t value) {
    for (uint32_t i = 0; i < engine->uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = UNIFORM_VEC2;
            engine->uniforms[i].value.vec2 = value;
            return;
        }
    }
    
    if (engine->uniform_count < MAX_UNIFORMS) {
        uniform_t* uniform = &engine->uniforms[engine->uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = UNIFORM_VEC2;
        uniform->value.vec2 = value;
        engine->uniform_count++;
    }
}

void uniform_set_vec3(engine_t* engine, const char* name, vec3_t value) {
    for (uint32_t i = 0; i < engine->uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = UNIFORM_VEC3;
            engine->uniforms[i].value.vec3 = value;
            return;
        }
    }
    
    if (engine->uniform_count < MAX_UNIFORMS) {
        uniform_t* uniform = &engine->uniforms[engine->uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = UNIFORM_VEC3;
        uniform->value.vec3 = value;
        engine->uniform_count++;
    }
}

void uniform_set_vec4(engine_t* engine, const char* name, vec4_t value) {
    for (uint32_t i = 0; i < engine->uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = UNIFORM_VEC4;
            engine->uniforms[i].value.vec4 = value;
            return;
        }
    }
    
    if (engine->uniform_count < MAX_UNIFORMS) {
        uniform_t* uniform = &engine->uniforms[engine->uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = UNIFORM_VEC4;
        uniform->value.vec4 = value;
        engine->uniform_count++;
    }
}

void uniform_set_int(engine_t* engine, const char* name, i32 value) {
    for (uint32_t i = 0; i < engine->uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = UNIFORM_INT;
            engine->uniforms[i].value.i = value;
            return;
        }
    }
    
    if (engine->uniform_count < MAX_UNIFORMS) {
        uniform_t* uniform = &engine->uniforms[engine->uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = UNIFORM_INT;
        uniform->value.i = value;
        engine->uniform_count++;
    }
}

void uniform_set_texture(engine_t* engine, const char* name, GLuint texture) {
    for (uint32_t i = 0; i < engine->uniform_count; i++) {
        if (strcmp(engine->uniforms[i].name, name) == 0) {
            engine->uniforms[i].type = UNIFORM_TEXTURE;
            engine->uniforms[i].value.texture = texture;
            return;
        }
    }
    
    if (engine->uniform_count < MAX_UNIFORMS) {
        uniform_t* uniform = &engine->uniforms[engine->uniform_count];
        strncpy(uniform->name, name, sizeof(uniform->name) - 1);
        uniform->type = UNIFORM_TEXTURE;
        uniform->value.texture = texture;
        engine->uniform_count++;
    }
}

void uniform_set_buffer_texture(engine_t* engine, const char* name, render_buffer_t* buffer) {
    uniform_set_texture(engine, name, buffer->texture);
}

void uniform_apply(engine_t* engine, shader_t* shader) {
    glUseProgram(shader->program);
    for (uint32_t i = 0; i < engine->uniform_count; i++) {
        uniform_t* uniform = &engine->uniforms[i];
        GLint location = glGetUniformLocation(shader->program, uniform->name); 
        if (location == -1) {
            continue;
        };
        switch (uniform->type) {
            case UNIFORM_FLOAT:
                glUniform1fv(location, 1, &uniform->value.f);
                break;
            case UNIFORM_VEC2:
                glUniform2fv(location, 1, &uniform->value.vec2.x);
                break;
            case UNIFORM_VEC3:
                glUniform3fv(location, 1, &uniform->value.vec3.x);
                break;
            case UNIFORM_VEC4:
                glUniform4fv(location, 1, &uniform->value.vec4.x);
                break;
            case UNIFORM_INT:
                glUniform1i(location, uniform->value.i);
                break;
            case UNIFORM_TEXTURE:
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, uniform->value.texture);
                glUniform1i(location, i);
                break;
        }
    }
}

// Utility functions
double get_time(void) {
    return glfwGetTime();
}

time_t get_file_mtime(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

char* load_file(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = malloc(size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, size, file);
    buffer[size] = '\0';
    
    fclose(file);
    return buffer;
}

void create_fullscreen_quad(GLuint* vao, GLuint* vbo) {
    f32 quad_vertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
    };
    
    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);
    
    glBindVertexArray(*vao);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2 * sizeof(f32)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

f32 vec3_length(vec3_t v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_normalize(vec3_t v) {
    f32 length = vec3_length(v);
    if (length > 0.0f) {
        v.x /= length;
        v.y /= length;
        v.z /= length;
    }
    return v;
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    vec3_t result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

// Static helper functions
static void key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
    engine_t* engine = (engine_t*)glfwGetWindowUserPointer(window);
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            engine->keys[key] = true;
        } else if (action == GLFW_RELEASE) {
            engine->keys[key] = false;
        }
    }
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    engine_t* engine = (engine_t*)glfwGetWindowUserPointer(window);
    engine->mouse_dx = xpos - engine->mouse_x;
    engine->mouse_dy = ypos - engine->mouse_y;
    engine->mouse_x = xpos;
    engine->mouse_y = ypos;
}

static void mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods) {
    engine_t* engine = (engine_t*)glfwGetWindowUserPointer(window);
    if (button >= 0 && button < 8) {
        if (action == GLFW_PRESS) {
            engine->mouse_buttons[button] = true;
        } else if (action == GLFW_RELEASE) {
            engine->mouse_buttons[button] = false;
        }
    }
}

static void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height) {
    engine_t* engine = (engine_t*)glfwGetWindowUserPointer(window);
    engine->window_width = width;
    engine->window_height = height;
    glViewport(0, 0, width, height);
}

static GLuint compile_shader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Shader compilation failed: %s\n", info_log);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

static GLuint create_shader_program(const char* vertex_source, const char* fragment_source) {
    GLuint vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    
    if (!vertex_shader || !fragment_shader) {
        if (vertex_shader) glDeleteShader(vertex_shader);
        if (fragment_shader) glDeleteShader(fragment_shader);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Shader program linking failed: %s\n", info_log);
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}


#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1024

// Simple zero-crossing frequency detection
float detect_frequency(float* buffer, int size, float sample_rate) {
    int zero_crossings = 0;
    
    for (int i = 1; i < size; i++) {
        if ((buffer[i-1] >= 0 && buffer[i] < 0) || (buffer[i-1] < 0 && buffer[i] >= 0)) {
            zero_crossings++;
        }
    }
    
    // Frequency is roughly zero crossings per second divided by 2
    float frequency = (zero_crossings * sample_rate) / (2.0f * size);
    return frequency;
}

// Calculate RMS volume
float calculate_volume(float* buffer, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += buffer[i] * buffer[i];
    }
    return sqrt(sum / size);
}

// Simple frequency band classification
void classify_frequency_bands(float frequency, float volume, float* low, float* mid, float* high) {
    *low = 0.0f;
    *mid = 0.0f;
    *high = 0.0f;
    
    if (frequency >= 20 && frequency <= 250) {
        *low = volume;
    } else if (frequency > 250 && frequency <= 4000) {
        *mid = volume;
    } else if (frequency > 4000 && frequency <= 20000) {
        *high = volume;
    }
}

// Audio

typedef struct {
    vec3_t amps;
    
    // internal
    b8 running;
    float* buffer;
    PaStream* stream;
    int buffer_size;
} audio_data_t;
static audio_data_t audio_data = {0};

// Audio callback function
static int audio_callback(const void* input_buffer, void* output_buffer,
                         unsigned long frames_per_buffer,
                         const PaStreamCallbackTimeInfo* time_info,
                         PaStreamCallbackFlags status_flags,
                         void* user_data) {
    
    audio_data_t* data = (audio_data_t*)user_data;
    const float* input = (const float*)input_buffer;
    
    if (input_buffer == NULL) {
        return paContinue;
    }
    
    // Copy input to buffer
    for (unsigned long i = 0; i < frames_per_buffer; i++) {
        data->buffer[i] = input[i];
    }
    
    float volume = calculate_volume(data->buffer, frames_per_buffer);
    float frequency = detect_frequency(data->buffer, frames_per_buffer, SAMPLE_RATE);
    
    float low_vol, mid_vol, high_vol;
    classify_frequency_bands(frequency, volume, &low_vol, &mid_vol, &high_vol);
    
    float volume_percent = volume * 10000.0f; 
    if (volume_percent > 100.0f) volume_percent = 100.0f;
    
    // Scale band volumes
    float low_percent = low_vol * 10000.0f;
    float mid_percent = mid_vol * 10000.0f;
    float high_percent = high_vol * 10000.0f;
    
    if (low_percent > 100.0f) low_percent = 100.0f;
    if (mid_percent > 100.0f) mid_percent = 100.0f;
    if (high_percent > 100.0f) high_percent = 100.0f;
    
    data->amps = (vec3_t){low_vol, mid_vol, high_vol};

    //printf("\rVol: %5.1f%% | Freq: %6.1fHz | Low: %5.1f%% | Mid: %5.1f%% | High: %5.1f%% ",
    //       volume_percent, frequency, low_percent, mid_percent, high_percent);
    //
    //// Visual bars
    //int vol_bars = (int)(volume_percent / 5);
    //printf("Vol:");
    //for (int i = 0; i < 20; i++) {
    //    if (i < vol_bars) printf("█");
    //    else printf("░");
    //}
    //
    //fflush(stdout);
    
    return paContinue;
}


void list_audio_devices() {
    int num_devices = Pa_GetDeviceCount();
    printf("Available audio devices:\n");
    
    for (int i = 0; i < num_devices; i++) {
        const PaDeviceInfo* device_info = Pa_GetDeviceInfo(i);
        const PaHostApiInfo* host_api_info = Pa_GetHostApiInfo(device_info->hostApi);
        
        printf("Device %d: %s (Host API: %s)\n", i, device_info->name, host_api_info->name);
        printf("  Max input channels: %d\n", device_info->maxInputChannels);
        printf("  Max output channels: %d\n", device_info->maxOutputChannels);
        printf("  Default sample rate: %.0f Hz\n", device_info->defaultSampleRate);
        printf("\n");
    }
}

void audio_init() {
    PaStreamParameters input_parameters;
    PaError err;
    
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        audio_cleanup();
        return;
    }
    
    audio_data.buffer = (float*)malloc(FRAMES_PER_BUFFER * sizeof(float));
    audio_data.buffer_size = FRAMES_PER_BUFFER;
    
    if (!audio_data.buffer) {
        printf("Memory allocation failed\n");
        audio_cleanup();
        return;
    }
    
    int input_device = Pa_GetDefaultInputDevice();
    if (input_device == paNoDevice) {
        printf("No input device found\n");
        audio_cleanup();
        return;
    }
    
    input_parameters.device = input_device;
    input_parameters.channelCount = 1;
    input_parameters.sampleFormat = paFloat32;
    input_parameters.suggestedLatency = Pa_GetDeviceInfo(input_device)->defaultLowInputLatency;
    input_parameters.hostApiSpecificStreamInfo = NULL;
    err = Pa_OpenStream(&audio_data.stream,
                        &input_parameters,
                        NULL,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        paClipOff,
                        audio_callback,
                        &audio_data);
    
    if (err != paNoError) {
        printf("Cannot open stream: %s\n", Pa_GetErrorText(err));
        audio_cleanup();
        return;
    }
    
    err = Pa_StartStream(audio_data.stream);
    if (err != paNoError) {
        printf("Cannot start stream: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(audio_data.stream);
        audio_cleanup();
        return;
    }
    
    printf("Simple Audio Monitor - Press Enter to stop\n");
    printf("Volume and frequency detection active...\n\n");
    
    audio_data.running = true;
}

void audio_cleanup(){
    audio_data.running = false;
    PaError err;
    err = Pa_StopStream(audio_data.stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
        goto cleanup;
    }

    err = Pa_CloseStream(audio_data.stream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

cleanup:
    free(audio_data.buffer);
    Pa_Terminate();
}


vec3_t audio_get_amplitudes(){
    return audio_data.amps;
}
