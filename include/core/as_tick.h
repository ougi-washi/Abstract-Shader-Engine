// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_array.h"

// TODO: thread it
typedef struct as_tick_handle as_tick_handle;
typedef struct as_tick_handle
{
    void (*func_ptr)(void*, const f64);
    void* arg;
    f32 tick_rate; // unused yet    
} as_tick_handle;

AS_ARRAY_DECLARE(as_tick_handles_1024, 1024, as_tick_handle);

typedef struct as_tick_system
{
    as_tick_handles_1024 handles;
    AS_DECLARE_TYPE;
}as_tick_system;

extern as_tick_handle* as_tick_handle_create(as_tick_system* tick_system);
extern void as_tick_handle_execute(const as_tick_handle* tick_handle, const f64 delta_time);

extern as_tick_system* as_tick_system_create();
extern void as_tick_system_destroy(as_tick_system* tick_system);
extern void as_tick_system_execute(const as_tick_system* tick_system, const f64 delta_time);
