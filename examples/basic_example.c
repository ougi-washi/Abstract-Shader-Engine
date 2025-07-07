#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "src/engine.h"

int main(int argc, char **argv) {
    const char *vert_path = "resources/vert.glsl";
    const char *frag_path = "resources/frag.glsl";
    const char *obj_path  = "resources/model.obj";

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "GLFW Engine", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (engine_init() != 0) {
        fprintf(stderr, "Engine initialization failed\n");
        return 1;
    }

    mesh_t *mesh = mesh_load_obj(obj_path);
    shader_program_t *shader = shader_create(vert_path, frag_path);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            shader_reload(shader);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_bind(shader);
        mesh_draw(mesh);
        shader_unbind();

        glfwSwapBuffers(window);
    }

    mesh_destroy(mesh);
    shader_destroy(shader);
    engine_shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
