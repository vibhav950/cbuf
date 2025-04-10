#include "cbuf.h"
#include "test_utils.h"

void test_init_free() {
  cbuf_t cbuf;

  /* Initialize with invalid size */
  TEST_ASSERT(cbuf_init(&cbuf, 10) == -1,
              "Should fail with size < CBUF_MIN_CAPACITY");

  /* Initialize with valid size */
  TEST_ASSERT(cbuf_init(&cbuf, CBUF_MIN_CAPACITY) == 0,
              "Valid initialization failed");
  TEST_ASSERT(cbuf.buf != NULL, "Buffer allocation failed");
  TEST_ASSERT(cbuf.capacity == CBUF_MIN_CAPACITY, "Incorrect capacity");
  TEST_ASSERT(atomic_load(&cbuf.readp) == cbuf.buf,
              "Read pointer not initialized");
  TEST_ASSERT(atomic_load(&cbuf.writep) == cbuf.buf,
              "Write pointer not initialized");

  cbuf_free(&cbuf);
  TEST_ASSERT(cbuf.capacity == 0, "Capacity not reset after free");
  TEST_ASSERT(atomic_load(&cbuf.readp) == NULL,
              "Read pointer not reset after free");
  TEST_ASSERT(atomic_load(&cbuf.writep) == NULL,
              "Write pointer not reset after free");
}

void test_make_release() {
  cbuf_t cbuf;
  uint8_t *buffer = malloc(CBUF_MIN_CAPACITY);
  uint8_t *returned_buffer = NULL;

  TEST_ASSERT(cbuf_make(&cbuf, buffer, CBUF_MIN_CAPACITY) == 0,
              "Valid make failed");
  TEST_ASSERT(cbuf.buf == buffer, "Buffer pointer not set correctly");
  TEST_ASSERT(cbuf.capacity == CBUF_MIN_CAPACITY, "Incorrect capacity");

  size_t capacity = cbuf_release(&cbuf, &returned_buffer);
  TEST_ASSERT(capacity == CBUF_MIN_CAPACITY, "Incorrect capacity returned");
  TEST_ASSERT(returned_buffer == buffer, "Wrong buffer returned");
  TEST_ASSERT(cbuf.capacity == 0, "Capacity not reset after release");

  free(buffer);
}

void test_empty_full_state() {
  cbuf_t cbuf;
  TEST_ASSERT(cbuf_init(&cbuf, CBUF_MIN_CAPACITY) == 0,
              "Initialization failed");

  TEST_ASSERT(cbuf_is_empty(&cbuf) != 0, "New buffer must be empty");
  TEST_ASSERT(cbuf_is_full(&cbuf) == 0, "New buffer should not be full");
  TEST_ASSERT(cbuf_get_readable_size(&cbuf) == 0,
              "New buffer should have 0 readable bytes");

  uint8_t data[100];
  memset(data, 0x42, 100);

  /* Write to nearly fill the buffer (capacity - 1) */
  size_t write_size = cbuf_get_capacity(&cbuf);
  TEST_ASSERT(write_size == CBUF_MIN_CAPACITY - 1,
              "Capacity calculation incorrect");

  /* Write in chunks until the buffer is full */
  size_t remaining = write_size;
  while (remaining > 0) {
    size_t chunk = remaining > 100 ? 100 : remaining;
    ssize_t written = cbuf_write_blocking(&cbuf, data, chunk, -1);
    TEST_ASSERT(written == chunk, "Failed to write to buffer");
    remaining -= written;
  }

  /* Buffer should now be full */
  TEST_ASSERT(cbuf_is_empty(&cbuf) == 0, "Buffer should not be empty");
  TEST_ASSERT(cbuf_is_full(&cbuf) > 0, "Buffer must be full");
  TEST_ASSERT(cbuf_get_readable_size(&cbuf) == write_size,
              "Buffer should have correct readable size");

  /* Read all the data */
  uint8_t read_data[CBUF_MIN_CAPACITY];
  ssize_t read = cbuf_read_blocking(&cbuf, read_data, write_size, -1, true);
  TEST_ASSERT(read == write_size, "Failed to read all data");

  /* Buffer must be empty */
  TEST_ASSERT(cbuf_is_empty(&cbuf) > 0, "Buffer must be empty after reading");
  TEST_ASSERT(cbuf_is_full(&cbuf) == 0,
              "Buffer should not be full after reading");

  cbuf_free(&cbuf);
}

void test_peek_and_remove() {
  cbuf_t cbuf;
  TEST_ASSERT(cbuf_init(&cbuf, CBUF_MIN_CAPACITY) == 0,
              "Initialization failed");

  /* Write some data */
  uint8_t write_data[100];
  for (int i = 0; i < 100; i++)
    write_data[i] = i;

  TEST_ASSERT(cbuf_write_blocking(&cbuf, write_data, 100, -1) == 100,
              "Failed to write test data");

  /* Peek */
  uint8_t peek_data[50];
  TEST_ASSERT(cbuf_peek(&cbuf, peek_data, 50) == 50, "Failed to peek data");
  for (int i = 0; i < 50; i++)
    TEST_ASSERT(peek_data[i] == i, "Peek data mismatch");

  /* Verify peek didn't consume the data */
  TEST_ASSERT(cbuf_get_readable_size(&cbuf) == 100,
              "Peek should not consume data");

  /* Remove the first half of the data */
  TEST_ASSERT(cbuf_remove(&cbuf, 50) == 50, "Failed to remove data");
  TEST_ASSERT(cbuf_get_readable_size(&cbuf) == 50,
              "Buffer size incorrect after remove");

  /* Peek again, verify what remains is the second half */
  TEST_ASSERT(cbuf_peek(&cbuf, peek_data, 50) == 50,
              "Failed to peek after remove");
  for (int i = 0; i < 50; i++)
    TEST_ASSERT(peek_data[i] == i + 50, "Peek data mismatch after remove");

  cbuf_free(&cbuf);
}

int main() {
  printf("Running basic tests...\n");

  test_init_free();
  printf("\x1B[92m  ✓ init/free tests passed\x1B[0m\n");

  test_make_release();
  printf("\x1B[92m  ✓ make/release tests passed\x1B[0m\n");

  test_empty_full_state();
  printf("\x1B[92m  ✓ empty/full state tests passed\x1B[0m\n");

  test_peek_and_remove();
  printf("\x1B[92m  ✓ peek/remove tests passed\x1B[0m\n");

  printf("All basic tests passed!\n");
  return 0;
}
