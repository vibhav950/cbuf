#include "cbuf_timeout.h"
#include "test_utils.h"

#include <math.h>

int main() {
  cbuf_t cbuf;
  int64_t start, end, diff;
  uint8_t *buf = malloc(1); // dummy buffer

  TEST_ASSERT(cbuf_init(&cbuf, CBUF_MIN_CAPACITY) == 0,
              "Failed to initialize buffer");

  int timeouts[] = {50, 100, 1000, 2000, 5000};

  for (int i = 0; i < ARR_COUNT(timeouts); ++i) {
    start = cbuf_time_now();

    cbuf_read_blocking(&cbuf, buf, sizeof(buf), timeouts[i], false);

    end = cbuf_time_now();
    diff = cbuf_time_diff(end, start);
    printf("timeout %d ms : delta %lld ms\n", timeouts[i],
           llabs(diff - timeouts[i]));
  }

  free(buf);
  cbuf_free(&cbuf);
  return 0;
}
