#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#if defined(__has_builtin) && __has_builtin(__builtin_expect)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

#define MIN(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))
#define MAX(a, b) ((a) ^ (((a) ^ (b)) & -((a) < (b))))