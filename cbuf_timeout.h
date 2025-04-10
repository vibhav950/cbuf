#pragma once

#include "defs.h"

#include <sys/time.h>
#include <time.h>

typedef struct cbuf_timeout_st {
  struct timeval begin;
  int64_t expire_in_usec;
} cbuf_timeout_t;

static inline __attribute__((always_inline)) void
cbuf_timeout_begin(cbuf_timeout_t *timeout, int64_t expire_in_usec) {
  if (likely(timeout)) {
    gettimeofday(&timeout->begin, NULL);
    timeout->expire_in_usec = expire_in_usec;
  }
}

static inline __attribute__((always_inline)) bool
cbuf_timeout_expired(cbuf_timeout_t *timeout) {
  struct timeval now, begin = timeout->begin;
  if (likely(timeout)) {
    if (timeout->expire_in_usec < 0)
      return false;

    gettimeofday(&now, NULL);
    int64_t diff = ((int64_t)now.tv_sec - begin.tv_sec)*1000000 +
                   (now.tv_usec - begin.tv_usec);
    if (diff >= timeout->expire_in_usec)
      return true;
    else
      return false;
  }
  return true;
}
