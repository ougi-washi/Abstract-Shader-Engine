// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#define AS_EDITOR 1

// Window
#define AS_ENGINE_WINDOW_WIDTH 1900
#define AS_ENGINE_WINDOW_HEIGHT 1080
#define AS_ENGINE_WINDOW_NAME "Abstract Shader Engine"

// PATH
#define AS_PATH_BIN "./"
#define AS_PATH_RESOURCES "../resources/"
#define AS_PATH_SHADERS "../resources/shaders/"
#define AS_PATH_TEXTURES "../resources/textures/"
#define AS_PATH_SCENES "../resources/scenes/"
#define AS_PATH_CACHED "../cached/"
#define AS_PATH_CACHED_SHADERS "../cached/shaders/"

// DEFAULT
#define AS_PATH_DEFAULT_VERT_SHADER "../resources/shaders/default_vertex.glsl"
#define AS_PATH_DEFAULT_FRAG_SHADER "../resources/shaders/default_fragment.glsl"
#define AS_PATH_EMPTY_GRAY_FRAG_SHADER "../resources/shaders/empty_gray_fragment.glsl"
#define AS_PATH_DEFAULT_TEXTURE "../resources/textures/default_texture.png"
#define AS_PATH_DEFAULT_SCENE "../resources/scenes/default_scene.as_scene"
#define AS_PATH_DEFAULT_UI_VERT_SHADER "../resources/shaders/ui/default_ui_vertex.glsl"
#define AS_PATH_DEFAULT_UI_FRAG_SHADER "../resources/shaders/ui/default_ui_fragment.glsl"

// Render
#define AS_MAX_SCENE_OBJECTS 1024
#define AS_MAX_SCENE_LIGHTS 128
#define AS_MAX_SCENE_CAMERAS 128
#define AS_MAX_SCREEN_OBJECTS 128
#define AS_MAX_TEXTURE_POOL_SIZE 512