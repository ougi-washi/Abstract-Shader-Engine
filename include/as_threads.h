// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include "as_array.h"

#if PLATFORM_WINDOWS
#include <windows.h>
typedef HANDLE as_thread;
typedef DWORD as_thread_id;
#elif PLATFORM_LINUX || PLATFORM_UNIX
#include <stdlib.h>
#include <pthreads.h>
typedef pthread_t as_thread;
typedef pid_t as_thread_id;
#endif


extern as_thread as_thread_create(void* (*func)(void*), void* arg);
extern void as_thread_join(as_thread thread);
extern as_thread_id as_thread_get_id();
extern void as_thread_terminate(as_thread thread);
extern i32 as_get_cpu_cores();
extern i32 as_thread_get_priority(as_thread thread);
extern void as_thread_set_priority(as_thread thread, const i32 priority);
extern void as_thread_detach(as_thread thread);


#if PLATFORM_WINDOWS
#define AS_MUTEX_TYPE             HANDLE
#define AS_MUTEX_INITIALIZER      NULL
#define AS_MUTEX_SETUP(x)         (x) = CreateMutex(NULL, FALSE, NULL)
#define AS_MUTEX_CLEANUP(x)       (CloseHandle(x) == 0)
#define AS_MUTEX_LOCK(x)          emulate_pthread_mutex_lock(&(x))
#define AS_MUTEX_UNLOCK(x)        (ReleaseMutex(x) == 0)
extern i32 emulate_pthread_mutex_lock(volatile AS_MUTEX_TYPE* mx);
#elif PLATFORM_LINUX || PLATFORM_UNIX
#define AS_MUTEX_TYPE             pthread_mutex_t
#define AS_MUTEX_INITIALIZER      PTHREAD_MUTEX_INITIALIZER
#define AS_MUTEX_SETUP(x)         pthread_mutex_init(&(x), NULL)
#define AS_MUTEX_CLEANUP(x)       (pthread_mutex_destroy(&(x)) == 0)
#define AS_MUTEX_LOCK(x)          (pthread_mutex_lock(&(x)) == 0)
#define AS_MUTEX_UNLOCK(x)        (pthread_mutex_unlock(&(x)) == 0)
#endif

typedef AS_MUTEX_TYPE as_mutex;
bool as_mutex_init(as_mutex* mutex);
bool as_mutex_lock(as_mutex* mutex);
bool as_mutex_unlock(as_mutex* mutex);
bool as_mutex_destroy(as_mutex* mutex);
