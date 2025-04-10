#pragma once

#include "cbuf.h"
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define TEST_ASSERT(expr, message)                                             \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n", __FILE__, __LINE__,   \
              message);                                                        \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
  } while (0)

/* Thread-safe counter for assertions */
typedef struct {
  int value;
  pthread_mutex_t mutex;
} atomic_counter_t;

static inline void counter_init(atomic_counter_t *counter) {
  counter->value = 0;
  pthread_mutex_init(&counter->mutex, NULL);
}

static inline void counter_destroy(atomic_counter_t *counter) {
  pthread_mutex_destroy(&counter->mutex);
}

static inline void counter_increment(atomic_counter_t *counter) {
  pthread_mutex_lock(&counter->mutex);
  counter->value++;
  pthread_mutex_unlock(&counter->mutex);
}

static inline void counter_add(atomic_counter_t *counter, int value) {
  pthread_mutex_lock(&counter->mutex);
  counter->value += value;
  pthread_mutex_unlock(&counter->mutex);
}

static inline int counter_get(atomic_counter_t *counter) {
  int val;
  pthread_mutex_lock(&counter->mutex);
  val = counter->value;
  pthread_mutex_unlock(&counter->mutex);
  return val;
}

/* Shared test context for producer-consumer tests */
typedef struct {
  cbuf_t *cbuf;
  size_t num_items;
  size_t item_size;
  int64_t timeout_usec;
  atomic_counter_t produced;
  atomic_counter_t consumed;
  bool producer_done;
  bool consumer_done;
} test_context_t;

static inline void test_context_init(test_context_t *ctx, cbuf_t *cbuf,
                                     size_t num_items, size_t item_size,
                                     int64_t timeout_usec) {
  ctx->cbuf = cbuf;
  ctx->num_items = num_items;
  ctx->item_size = item_size;
  ctx->timeout_usec = timeout_usec;
  counter_init(&ctx->produced);
  counter_init(&ctx->consumed);
  ctx->producer_done = false;
  ctx->consumer_done = false;
}

static inline void test_context_destroy(test_context_t *ctx) {
  counter_destroy(&ctx->produced);
  counter_destroy(&ctx->consumed);
  // Ensure no double-free occurs
  ctx->cbuf = NULL;
}