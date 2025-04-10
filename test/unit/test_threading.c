#include "cbuf.h"
#include "test_utils.h"
#include <pthread.h>

void *producer_thread(void *arg) {
  test_context_t *ctx = (test_context_t *)arg;
  uint8_t *data = malloc(ctx->item_size);

  for (size_t i = 0; i < ctx->num_items; i++) {
    /* Fill with a pattern based on the item number */
    for (size_t j = 0; j < ctx->item_size; j++)
      data[j] = (i + j) & 0xFF;

    ssize_t written =
        cbuf_write_blocking(ctx->cbuf, data, ctx->item_size, ctx->timeout_usec);
    if (written == ctx->item_size) {
      counter_increment(&ctx->produced);
    } else {
      printf("Producer: Failed to write item %zu (wrote %zd bytes)\n", i,
             written);
    }
  }

  ctx->producer_done = true;
  free(data);
  return NULL;
}

void *consumer_thread(void *arg) {
  test_context_t *ctx = (test_context_t *)arg;
  uint8_t *data = malloc(ctx->item_size);

  for (size_t i = 0; i < ctx->num_items; i++) {
    ssize_t read = cbuf_read_blocking(ctx->cbuf, data, ctx->item_size,
                                      ctx->timeout_usec, true);
    if (read == ctx->item_size) {
      /* Verify the pattern */
      bool data_valid = true;
      for (size_t j = 0; j < ctx->item_size; j++) {
        if (data[j] != ((i + j) & 0xFF)) {
          data_valid = false;
          printf("Consumer: Data mismatch at item %zu, byte %zu: expected %d, "
                 "got %d\n",
                 i, j, (i + j) & 0xFF, data[j]);
          break;
        }
      }

      if (data_valid)
        counter_increment(&ctx->consumed);
    } else {
      printf("Consumer: Failed to read item %zu (read %zd bytes)\n", i, read);
    }
  }

  ctx->consumer_done = true;
  free(data);
  return NULL;
}

void test_producer_consumer() {
  cbuf_t cbuf;
  test_context_t ctx;
  pthread_t producer, consumer;

  TEST_ASSERT(cbuf_init(&cbuf, 4096) == 0, "Failed to initialize buffer");

  /* 1000 items, 64 bytes each, infinite timeout */
  test_context_init(&ctx, &cbuf, 1000, 64, -1);

  pthread_create(&producer, NULL, producer_thread, &ctx);
  pthread_create(&consumer, NULL, consumer_thread, &ctx);

  pthread_join(producer, NULL);
  pthread_join(consumer, NULL);

  printf("Producer completed: %d items produced\n", counter_get(&ctx.produced));
  printf("Consumer completed: %d items consumed\n", counter_get(&ctx.consumed));

  TEST_ASSERT(counter_get(&ctx.produced) == ctx.num_items,
              "Producer didn't produce all items");
  TEST_ASSERT(counter_get(&ctx.consumed) == ctx.num_items,
              "Consumer didn't consume all items");

  test_context_destroy(&ctx);
  cbuf_free(&cbuf);
}

void test_parallel_operations() {
  cbuf_t cbuf;
  TEST_ASSERT(cbuf_init(&cbuf, 1024) == 0, "Failed to initialize buffer");

  /* Create short-lived producer and consumer with a small number of items */
  test_context_t ctx;
  /* 10 items, 8 bytes each, infinite timeout */
  test_context_init(&ctx, &cbuf, 10, 8, -1);

  pthread_t producer, consumer;
  pthread_create(&producer, NULL, producer_thread, &ctx);
  pthread_create(&consumer, NULL, consumer_thread, &ctx);

  pthread_join(producer, NULL);
  pthread_join(consumer, NULL);

  TEST_ASSERT(cbuf_is_empty(&cbuf) > 0,
              "Buffer must be empty after equal production and consumption");

  test_context_destroy(&ctx);
  cbuf_free(&cbuf);
}

int main() {
  printf("Running threading tests...\n");

  test_producer_consumer();
  printf("\x1B[92m  ✓ Producer-consumer test passed\x1B[0m\n");

  test_parallel_operations();
  printf("\x1B[92m  ✓ Parallel operations test passed\x1B[0m\n");

  printf("All threading tests passed!\n");
  return 0;
}
