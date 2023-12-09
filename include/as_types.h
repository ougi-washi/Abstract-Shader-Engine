// Abstract Shader Engine - Jed Fakhfekh - https://github.com/ougi-washi

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "defines/as_errors.h"

// Base types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

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
#define AS_LOG(level, text)																				\
	if (level == LV_LOG)				printf("LOG : %s\n", text);											\
	else if(level == LV_WARNING)		printf("[%s|%d] WARNING: %s\n", __FILE__, __LINE__, text);			\
	else if(level == LV_ERROR)			printf("[%s|%d] ERROR: %s\n", __FILE__, __LINE__, text);

#define AS_ASSERT(result, text)																			\
if (!result) { AS_LOG(LV_ERROR, text); assert(result); }

#else
#define AS_LOG(level, text) {};
#endif