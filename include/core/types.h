#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream> // STD HAS BAD COMPILE TIME

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

// Error handling
#define CRASH_ON_ERROR false

enum log_level : u8 { LV_LOG = 0, LV_WARNING = 1, LV_ERROR = 2 };

#define AS_ASSERT(result, text) \
if (!result) { std::cout << "ERROR: " << text << std::endl; assert(result); }\

#if defined(_DEBUG) || defined(NDEBUG) || defined(DEBUG)
#define AS_LOG(level, text) \
	if (level == log_level::LV_LOG) { std::cout << "LOG: " << text << std::endl; }\
	else if(level == log_level::LV_WARNING) { std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "WARNING: " << text << std::endl; }\
	else if(level == log_level::LV_ERROR) { std::cout << "[" << __FILE__ << "|" << __LINE__ << "]" << "ERROR: " << text << std::endl;\
	if (CRASH_ON_ERROR) { fprintf(stderr, "Failure at %u %s\n", __LINE__, __FILE__); exit(-1); } }
#else
#define AS_LOG(level, text) {};
#endif