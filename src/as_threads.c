// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#include "as_threads.h"
#include "as_memory.h"

as_thread as_thread_create(void* (*func)(void*), void* arg)
{
	as_thread thread;
#ifdef PLATFORM_WINDOWS
	DWORD threadId;
	thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, arg, 0, &threadId);
#elif PLATFORM_LINUX || PLATFORM_UNIX
	pthread_create(&thread, NULL, func, arg);
#endif
	return thread;
}

void as_thread_join(as_thread thread)
{
#ifdef PLATFORM_WINDOWS
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
#elif PLATFORM_LINUX || PLATFORM_UNIX
	pthread_join(thread, NULL);
#endif
}

as_thread_id as_thread_get_id()
{
#if PLATFORM_WINDOWS
	return GetCurrentThreadId();
#elif PLATFORM_LINUX || PLATFORM_UNIX
	return syscall(SYS_gettid);
#endif
}

void as_thread_terminate(as_thread thread)
{
#ifdef PLATFORM_WINDOWS
	TerminateThread(thread, 0);
	CloseHandle(thread);
#elif PLATFORM_LINUX || PLATFORM_UNIX
	pthread_cancel(thread);
#endif
}

i32 as_get_cpu_cores()
{
#ifdef PLATFORM_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#elif PLATFORM_LINUX || PLATFORM_UNIX
	return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

i32 as_thread_get_priority(as_thread thread)
{
#if PLATFORM_WINDOWS
	return GetThreadPriority(thread);
#elif PLATFORM_LINUX || PLATFORM_UNIX
	struct sched_param param;
	i32 policy = SCHED_OTHER;
	i32 ret = pthread_getschedparam(thread, &policy, &param);
	if (ret == 0)
	{
		return param.sched_priority;
	}
#endif
	return -1;
}

void as_thread_set_priority(as_thread thread, const i32 priority)
{
#if PLATFORM_WINDOWS
	SetThreadPriority(thread, priority);
#elif PLATFORM_LINUX || PLATFORM_UNIX
	struct sched_param param;
	param.sched_priority = priority;
	pthread_setschedparam(thread, SCHED_OTHER, &param);
#endif
}

void as_thread_detach(as_thread thread)
{
#if PLATFORM_WINDOWS
	AS_LOG(LV_WARNING, "Cannot detach thread on windows, make sure there are no memory leaks");
#elif PLATFORM_LINUX || PLATFORM_UNIX
	pthread_detach(thread);
#endif
}

i32 emulate_pthread_mutex_lock(volatile AS_MUTEX_TYPE* mx)
{
	if (*mx == NULL) /* static initializer? */
	{
		HANDLE p = CreateMutex(NULL, FALSE, NULL);
		if (InterlockedCompareExchangePointer((PVOID*)mx, (PVOID)p, NULL) != NULL)
			CloseHandle(p);
	}
	return WaitForSingleObject(*mx, INFINITE) == WAIT_FAILED;
}

bool as_mutex_init(as_mutex* mutex)
{
	//mutex = AS_MALLOC_SINGLE(as_mutex);
	AS_MUTEX_SETUP(*mutex);
	return true;  
}

bool as_mutex_lock(as_mutex* mutex)
{
	return AS_MUTEX_LOCK(*mutex);
}

bool as_mutex_unlock(as_mutex* mutex)
{
	return AS_MUTEX_UNLOCK(*mutex);
}

bool as_mutex_destroy(as_mutex* mutex)
{
	bool result = AS_MUTEX_CLEANUP(*mutex);
	AS_FREE(mutex);
	return result;
}
