#pragma once

#include "defs.h"

#if defined(__linux__)
#include <time.h>
#elif defined(_WIN32)
#include <Windows.h>
#else
#error "unknown platform"
#endif

typedef struct cbuf_timeout_st {
  int64_t begin;          /* timeout begin (ms) */
  int64_t expire_in_msec; /* milliseconds till expiry */
} cbuf_timeout_t;

#ifdef DEBUG
/**[DEBUG]
 * Runtime platform check for a monotonically increasing high-resolution clock.
 */
void have_highres_mono_clock(void) {
#ifdef __linux__
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
    exit(EXIT_FAILURE);
#else /* _WIN32 */
  LARGE_INTEGER foo;
  if ((QueryPerformanceCounter(&foo) == 0) ||
      (QueryPerformanceFrequency(&foo) == 0)) {
    exit(EXIT_FAILURE);
  }
#endif
}
#endif /* DEBUG */

/**
 * cbuf_time_now()
 *
 * @brief Get current time in milleseconds.
 */

#ifdef __linux__
#define cbuf_time_now()                                                        \
  ({                                                                           \
    struct timespec ts;                                                        \
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);                                 \
    int64_t now = (int64_t)ts.tv_sec*1000 + ts.tv_nsec/1000000;                \
    (now);                                                                     \
  })
#else /* _MSC_VER */
#define cbuf_time_now()                                                        \
  ({                                                                           \
    LARGE_INTEGER now, freq;                                                   \
    (void)QueryPerformanceFrequency(&freq);                                    \
    (void)QueryPerformanceCounter(&now);                                       \
    int64_t diff = (1000LL * now.QuadPart) / freq.QuadPart;                    \
    (diff);                                                                    \
  })
#endif

/**
 * cbuf_time_diff(new, old)
 *
 * @brief Get time elapsed in milliseconds since @p old up until @p new.
 */
#define cbuf_time_diff(new, old) ((new) - (old))

/**
 * cbuf_timeout_init(timeout, expire_in_msec)
 *
 * @brief Set a timeout to expire in @p expire_in_msec milliseconds.
 */
INLINE
void cbuf_timeout_begin(cbuf_timeout_t *timeout, int64_t expire_in_msec) {
  if (likely(timeout)) {
    timeout->begin = cbuf_time_now();
    timeout->expire_in_msec = expire_in_msec;
  }
}

/**
 * cbuf_timeout_expired(timeout)
 *
 * @brief Check if the timeout has expired.
 */
INLINE bool cbuf_timeout_expired(cbuf_timeout_t *timeout) {
  uint64_t now;
  if (likely(timeout)) {
    if (timeout->expire_in_msec < 0)
      return false;
    now = cbuf_time_now();
    return cbuf_time_diff(now, timeout->begin) >= timeout->expire_in_msec;
  }
  return true;
}
