// Abstract-Shader-Engine - Ougi Washi

#ifndef AS_TYPES_H
#define AS_TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

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

typedef struct { f32 m[16]; } as_mat4;
typedef struct { f32 x, y, z; } as_vec3;
typedef struct { f32 x, y, z, w; } as_vec4;
typedef struct { f32 x, y; } as_vec2;

#ifndef RESOURCES_DIR
#  error "RESOURCES_DIR not defined!"
#endif

#define MAX_PATH_LENGTH 256

#endif // AS_TYPES_H
