#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#if !defined(__GNUC__) && !defined(__clang__) && !defined(_MSC_VER)
#error "unknown platform"
#endif

#if defined(__linux__)
#include <sched.h> // sched_yield()
#elif defined(_WIN32)
#include <Windows.h> // SwitchToThread()
#else
#error "unknown platform"
#endif

#ifndef __has_builtin /* < GCC 10 didn't have this */
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_expect)
#define unlikely(expr) __builtin_expect(!!(expr), 0)
#define likely(expr) __builtin_expect(!!(expr), 1)
#else
#define unlikely(expr) (expr)
#define likely(expr) (expr)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define INLINE static inline __attribute__((always_inline))
#else
/* _MSC_VER */
#define INLINE static inline __forceinline
#endif

#define MIN(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))
#define MAX(a, b) ((a) ^ (((a) ^ (b)) & -((a) < (b))))

#if ((defined(__GNUC__) && !defined(__clang__)) &&                             \
     (defined(__i386__) || defined(__x86_64__))) ||                            \
    (__has_builtin(__builtin_ia32_pause))
#define spin_pause() __builtin_ia32_pause()
#elif defined(__x86_64__) || defined(__i386__)
#if defined(_MSC_VER)
/*
#include <intrin.h>
#pragma intrinsic(_mm_pause())
#define spin_pause() _mm_pause()
*/
#define spin_pause() __asm volatile("pause")
#else
#define spin_pause()                                                           \
  do {                                                                         \
    __asm__ volatile("pause\n" ::: "memory");                                  \
  } while (0)
#endif
#else
#error "unknown platform"
#endif

/**
 * spin_yield()
 * @brief Attempt to make this thread relinquish the processor
 *
 * - On Linux, `sched_yield()` always succeeds so there is nothing we can do
 *   (afaik) if there are no other threads waiting on the current processor
 *
 * - On Windows, we sleep for 1ms to try saving power if execution is not
 *   switched to another thread
 */

#if defined(__linux__)
#define spin_yield() sched_yield()
#elif defined(_WIN32)
#define spin_yield()                                                           \
  do {                                                                         \
    if (SwitchToThread() == 0)                                                 \
      Sleep(1);                                                                \
  } while (0)
#endif

/**
 * decaying_sleep(pause, pause32)
 * @brief Sleep with an exponential/multiplicative backoff
 *
 * This macro is a little misleading in that it doesn't always actually "sleep".
 * Instead, it gradually shifts from short (less) pauses to longer (more) pauses
 * to finally `yield`ing the processor on each spin. In theory this should
 * adjust the busy-wait for both short and long waiting periods, where the
 * thread doesn't steal and burn resources from other threads waiting on the
 * same processor.
 *
 * Reference: https://github.com/gstrauss/plasma/blob/master/plasma_spin.c
 */
#define decaying_sleep(pause, pause32)                                         \
  do {                                                                         \
    if (likely(pause)) {                                                       \
      spin_pause();                                                            \
      pause--;                                                                 \
    } else if (likely(pause32)) {                                              \
      int i = 4;                                                               \
      do {                                                                     \
        spin_pause();                                                          \
        spin_pause();                                                          \
        spin_pause();                                                          \
        spin_pause();                                                          \
        spin_pause();                                                          \
        spin_pause();                                                          \
        spin_pause();                                                          \
        spin_pause();                                                          \
      } while (--i);                                                           \
      pause32--;                                                               \
    } else {                                                                   \
      spin_yield();                                                            \
    }                                                                          \
  } while (0)
