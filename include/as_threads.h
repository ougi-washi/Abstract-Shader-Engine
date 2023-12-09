// MMO_ECS_SERVER - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include "as_types.h"
#include <stdio.h>

#if PLATFORM_WINDOWS
#include <windows.h>
typedef HANDLE as_thread;
typedef DWORD as_thread_id;
#elif PLATFORM_LINUX || PLATFORM_UNIX
#include <pthreads.h>
typedef pthread_t as_thread;
typedef pid_t as_thread_id;
#endif

as_thread as_create_thread(void* (*func)(void*), void* arg);
void as_join_thread(as_thread thread);
as_thread_id as_get_thread_id();
void as_terminate_thread(as_thread thread);
i32 as_get_cpu_cores();
i32 as_get_thread_priority(as_thread thread);
void as_set_thread_priority(as_thread thread, const i32 priority);
void as_detach_thread(as_thread thread);