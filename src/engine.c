#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct mesh_t {
    GLuint vao, vbo;
    size_t vertex_count;
};

struct shader_program_t {
    GLuint program;
    char *vert_path, *frag_path;
};

int engine_init(void) {
    glEnable(GL_DEPTH_TEST);
    return 0;
}

static char *read_file(const char *path) {
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

static GLuint compile_shader(GLenum type, const char *src) {
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);
    GLint ok;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetShaderInfoLog(id, 4096, NULL, log);
        fprintf(stderr, "shader compile error (%s): %s\n",
                type == GL_VERTEX_SHADER ? "VERT" : "FRAG", log);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

shader_program_t *shader_create(const char *vert_path, const char *frag_path) {
    char *vsrc = read_file(vert_path);
    char *fsrc = read_file(frag_path);
    if (!vsrc || !fsrc) return NULL;
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fsrc);
    free(vsrc); free(fsrc);
    if (!vs || !fs) return NULL;

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetProgramInfoLog(prog, 4096, NULL, log);
        fprintf(stderr, "program link error: %s\n", log);
        glDeleteProgram(prog);
        return NULL;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    shader_program_t *s = malloc(sizeof(*s));
    s->program = prog;
    s->vert_path = strdup(vert_path);
    s->frag_path = strdup(frag_path);
    return s;
}

int shader_reload(shader_program_t *s) {
    GLuint old_prog = s->program;
    shader_program_t *new_s = shader_create(s->vert_path, s->frag_path);
    if (!new_s) return -1;
    s->program = new_s->program;
    free(s->vert_path); free(s->frag_path);
    s->vert_path = new_s->vert_path;
    s->frag_path = new_s->frag_path;
    free(new_s);
    glDeleteProgram(old_prog);
    return 0;
}

void shader_bind(const shader_program_t *s) {
    glUseProgram(s->program);
}

void shader_unbind(void) {
    glUseProgram(0);
}

void shader_destroy(shader_program_t *s) {
    if (!s) return;
    glDeleteProgram(s->program);
    free(s->vert_path);
    free(s->frag_path);
    free(s);
}

mesh_t *mesh_load_obj(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); return NULL; }
    float *verts = NULL;
    size_t cap = 0, vc = 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "v ", 2) == 0) {
            float x, y, z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            if (vc + 3 > cap) {
                cap = cap ? cap * 2 : 1024;
                verts = realloc(verts, cap * sizeof(float));
            }
            verts[vc++] = x;
            verts[vc++] = y;
            verts[vc++] = z;
        }
    }
    fclose(f);

    mesh_t *m = malloc(sizeof(*m));
    m->vertex_count = vc / 3;
    glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);
    glGenBuffers(1, &m->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, vc * sizeof(float), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    free(verts);
    return m;
}

void mesh_draw(const mesh_t *m) {
    glBindVertexArray(m->vao);
    glDrawArrays(GL_TRIANGLES, 0, m->vertex_count);
    glBindVertexArray(0);
}

void mesh_destroy(mesh_t *m) {
    if (!m) return;
    glDeleteBuffers(1, &m->vbo);
    glDeleteVertexArrays(1, &m->vao);
    free(m);
}

void engine_shutdown(void) {
    // no global state
}
