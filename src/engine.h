/* engine.h */
#ifndef ENGINE_H
#define ENGINE_H

#include <GL/gl.h>
#include <stddef.h>

typedef struct shader_program_t shader_program_t;
typedef struct mesh_t mesh_t;

int engine_init(void);
mesh_t *mesh_load_obj(const char *path);
void mesh_destroy(mesh_t *mesh);

shader_program_t *shader_create(const char *vert_path, const char *frag_path);
void shader_bind(const shader_program_t *shader);
void shader_unbind(void);
int shader_reload(shader_program_t *shader);
void shader_destroy(shader_program_t *shader);

void mesh_draw(const mesh_t *mesh);

void engine_shutdown(void);

#endif // ENGINE_H
