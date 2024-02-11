// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

// Base types
typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;
typedef float		f32;
typedef double		f64;
typedef size_t		sz;


// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define PLATFORM_WINDOWS 1
#ifndef _WIN64
#error "64-bit is required on Windows"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
// Linux OS
#define PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
// Catch anything not caught by the above.
#define PLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
// POSIX
#define PLATFORM_POSIX 1
#elif __APPLE__
// Apple platforms
#define PLATFORM_APPLE 1
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define PLATFORM_IOS 1
#define PLATFORM_IOS_SIMULATOR 1
#elif TARGET_OS_IPHONE
#define PLATFORM_IOS 1
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#else
#error "Unknown platform"
#endif

// Variables Management
#define AS_INIT(_type, _struct)						\
memset(_struct, 0, sizeof(_type));
#define AS_INIT_ARRAY(_type, _struct, _array_size)		\
memset(_struct, 0, sizeof(_type) * _array_size);
#define AS_CHECK_VAL(_value, _min, _max) ((_value > _min) && (_value < _max))
#define AS_ARRAY_SIZE(_array) sizeof(_array) / sizeof(_array[0]) 

// LOG & ERROR handling
#define CRASH_ON_ERROR 0

#if PLATFORM_WINDOWS
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
HANDLE printf_mutex;
#elif PLATFORM_LINUX || PLATFORM_UNIX
#include <unistd.h>
#include <sys/syscall.h>
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#if PLATFORM_WINDOWS
#define START_LOG() WaitForSingleObject(printf_mutex, INFINITE);
#define END_LOG() ReleaseMutex(printf_mutex);
#elif PLATFORM_LINUX || PLATFORM_UNIX
#define START_LOG() pthread_mutex_lock(&printf_mutex);
#deifne END_LOG() pthread_mutex_unlock(&printf_mutex);
#endif

typedef enum log_level { LV_LOG = 0, LV_WARNING = 1, LV_ERROR = 2 } log_level;

#if defined(_DEBUG) || defined(NDEBUG) || defined(DEBUG)
#define AS_LOG(level, text)																					\
	if (level == LV_LOG)				printf("LOG : %s\n", text);											\
	else if(level == LV_WARNING)		printf("[%s|%d] WARNING: %s\n", __FILE__, __LINE__, text);			\
	else if(level == LV_ERROR)			printf("[%s|%d] ERROR: %s\n", __FILE__, __LINE__, text);

#define AS_ASSERT(result, text)																				\
if (!result) { AS_LOG(LV_ERROR, text); assert(result); }

#else
#define AS_LOG(level, text) {};
#define AS_ASSERT(result, text)	{};
#endif

typedef enum as_flag
{
	AS_INVALID			= 0x00,
	AS_VALID			= 0x01,
	AS_LOCKED			= 0x02,
	AS_MAX				= 0x03,
} as_flag;

#define AS_FLAG as_flag obj_flag

#define AS_IS_VALID(_obj)      	(_obj && (u8)_obj->obj_flag >= AS_VALID && (u8)_obj->obj_flag < AS_MAX)
#define AS_IS_INVALID(_obj)    	(!(&_obj->obj_flag) || !AS_IS_VALID(_obj))
#define AS_IS_LOCKED(_obj)     	((u8)_obj->obj_flag == AS_LOCKED)
#define AS_IS_UNLOCKED(_obj)   	((u8)_obj->obj_flag == AS_VALID)

#define AS_SET_VALID(_obj)     	if(AS_IS_INVALID(_obj))	_obj->obj_flag = AS_VALID
#define AS_SET_INVALID(_obj)	_obj->obj_flag = AS_INVALID
#define AS_LOCK(_obj)    	if(AS_IS_UNLOCKED(_obj))	_obj->obj_flag = AS_LOCKED
#define AS_UNLOCK(_obj)  	if(AS_IS_LOCKED(_obj))		_obj->obj_flag = AS_VALID

#define AS_WAIT_AND_LOCK(_obj) 																\
u64 loop_counter = 0; 																		\
while (AS_IS_LOCKED(_obj) && loop_counter < 9000000) { loop_counter++; };					\
AS_LOCK(_obj)
