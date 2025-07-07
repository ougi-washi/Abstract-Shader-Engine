/* engine.h */
#ifndef ENGINE_H
#define ENGINE_H

#include <GL/gl.h>
#include <stddef.h>

// Opaque handle to a shader program
typedef struct shader_program_t shader_program_t;
// Opaque handle to a mesh loaded from .obj
typedef struct mesh_t mesh_t;

// Initialize the engine (GL context must be created by caller)
int engine_init(void);

// Load .obj file into a mesh_t (positions only)
mesh_t *mesh_load_obj(const char *path);
// Free mesh resources
void mesh_destroy(mesh_t *mesh);

// Create shader program from GLSL source files (vertex + fragment)
shader_program_t *shader_create(const char *vert_path, const char *frag_path);
// Bind shader for rendering
void shader_bind(const shader_program_t *shader);
// Unbind any shader
void shader_unbind(void);
// Reload shader sources from disk and recompile
int shader_reload(shader_program_t *shader);
// Free shader
void shader_destroy(shader_program_t *shader);

// Draw a mesh with bound shader
void mesh_draw(const mesh_t *mesh);

// Cleanup engine resources
void engine_shutdown(void);

#endif // ENGINE_H
