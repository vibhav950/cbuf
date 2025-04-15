# cbuf - SPSC circular (ring) buffer in C

This implementation is thread-safe for a single producer and single consumer.

[![Static Badge](https://img.shields.io/badge/Unlicense-blue?style=plastic&logo=unlicense&logoColor=white&logoSize=auto&label=license&labelColor=grey&color=blue)](https://github.com/vibhav950/cbuf/blob/main/LICENSE) ![Static Badge](https://img.shields.io/badge/--std%3Dc11-blue?style=plastic&logo=c&logoColor=white&logoSize=auto&labelColor=grey&color=blue)

## Features

- No expensive mutex locks - cbuf uses C11 atomics for sequential consistency
- Built-in millisecond resolution timeouts for bounded waits (blocking read/write)

## API Reference

### cbuf initialization

| Function                                                | Usage                                                                                                                                                                                      |
| ------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `int cbuf_init(cbuf_t *cbuf, size_t capacity)`          | • Allocates memory for a circular buffer with the specified capacity<br>• Returns 0 on success, -1 on failure<br>• Capacity must be between `CBUF_MIN_CAPACITY` and `CBUF_MAX_CAPACITY`    |
| `void cbuf_free(cbuf_t *cbuf)`                          | • Frees the memory allocated for the circular buffer<br>• Not thread safe                                                                                                                  |
| `int cbuf_make(cbuf_t *cbuf, uint8_t *buf, size_t len)` | • Initializes a circular buffer using an externally provided buffer<br>• Returns 0 on success, -1 on failure<br>• Buffer ownership transfers to the cbuf                                   |
| `size_t cbuf_release(cbuf_t *cbuf, uint8_t **buf)`      | • Releases the buffer from the circular buffer for external use<br>• Returns the capacity of the released buffer<br>• Transfers ownership of `buf` back to the caller<br>• Not thread safe |
| `size_t cbuf_get_capacity(cbuf_t *cbuf)`                | • Returns the capacity of the circular buffer                                                                                                                                              |

### cbuf state queries

| Function                                                                       | Usage                                                                                                                                                                                                                            |
| ------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `int cbuf_is_empty(cbuf_t *cbuf)`                                              | • Returns >0 if empty, 0 if not empty, -1 for invalid arguments<br>• Result may be stale due to concurrent nature                                                                                                                |
| `int cbuf_is_full(cbuf_t *cbuf)`                                               | • Returns >0 if full, 0 if not full, -1 for invalid arguments<br>• Result may be stale due to concurrent nature                                                                                                                  |
| `ssize_t cbuf_get_readable_size(cbuf_t *cbuf)`                                 | • Returns the number of bytes available to read, or -1 for invalid arguments<br>• Result may be stale due to concurrent nature                                                                                                   |
| `int cbuf_waitfor_readable(cbuf_t *cbuf, size_t nbytes, int64_t timeout_msec)` | • Waits until at least nbytes are available to read or timeout occurs<br>• Returns >0 when data is available, 0 on timeout, -1 for invalid arguments<br>• Special timeout values: 0 (return immediately), -1 (wait indefinitely) |

### cbuf data operations

| Function                                                                                                | Usage                                                                                                                                                                                                                                                                                                                                    |
| ------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ssize_t cbuf_write_blocking(cbuf_t *cbuf, const uint8_t *buf, size_t nbytes, int64_t timeout_msec)`    | • Writes data to the buffer, blocking until space is available or timeout occurs<br>• Returns the number of bytes written, or -1 for invalid arguments<br>• Special timeout values: 0 (return immediately), -1 (wait indefinitely)                                                                                                       |
| `ssize_t cbuf_read_blocking(cbuf_t *cbuf, uint8_t *buf, size_t nbytes, int64_t timeout_msec, bool all)` | • Reads data from the buffer (FIFO ordering), blocking until data is available or timeout occurs<br>• Set `all` to true to wait for all requested bytes or false to read what's available<br>• Returns the number of bytes read, or -1 for invalid arguments<br>• Special timeout values: 0 (return immediately), -1 (wait indefinitely) |
| `ssize_t cbuf_peek(cbuf_t *cbuf, uint8_t *buf, size_t nbytes)`                                          | • Reads data from the buffer without consuming it (FIFO ordering)<br>• Returns the number of bytes read, or -1 for invalid arguments                                                                                                                                                                                                     |
| `ssize_t cbuf_remove(cbuf_t *cbuf, size_t nbytes)`                                                      | • Removes (consumes) data from the buffer without reading it<br>• Returns the number of bytes removed, or -1 for invalid arguments                                                                                                                                                                                                       |

## Run tests

Build and run tests using CMake:

```zsh
git clone https://github.com/vibhav950/cbuf.git
cd cbuf
mkdir build && cd build
cmake ..
make
ctest -V
```

## Notes

This library requries C11 atomics to be enabled. From what I can tell, this feature is still labelled
"experimental" up until Visual Studio 17.5, so there is no support for MSVC.

## License

This project is licensed under The Unlicense.
