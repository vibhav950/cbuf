# cbuf - SPSC circular (ring) buffer in C

This implementation is thread-safe for a single producer and single consumer.

## Features

- No expensive mutex locks - cbuf uses C11 atomics for sequential consistency
- Built-in millisecond resolution timeouts for bounded waits (blocking read/write)

## API Documentation

### Buffer Initialization and Management

#### `int cbuf_init(cbuf_t *cbuf, size_t capacity)`

Allocates memory for a circular buffer with the specified capacity.

- Returns 0 on success, -1 on failure
- Capacity must be between CBUF_MIN_CAPACITY and CBUF_MAX_CAPACITY

#### `void cbuf_free(cbuf_t *cbuf)`

Frees the memory allocated for the circular buffer.

#### `int cbuf_make(cbuf_t *cbuf, uint8_t *buf, size_t len)`

Initializes a circular buffer using an externally provided buffer.

- Returns 0 on success, -1 on failure

#### `size_t cbuf_release(cbuf_t *cbuf, uint8_t **buf)`

Releases the buffer from the circular buffer for external use, transferring ownership to the caller.

- Returns the capacity of the released buffer
- The cbuf cannot be used again until re-initialized

#### `size_t cbuf_get_capacity(cbuf_t *cbuf)`

Returns the capacity of the circular buffer.

### Buffer State Queries

#### `int cbuf_is_empty(cbuf_t *cbuf)`

Checks if the buffer is empty.

- Returns >0 if empty, 0 if not empty, -1 for invalid arguments

#### `int cbuf_is_full(cbuf_t *cbuf)`

Checks if the buffer is full.

- Returns >0 if full, 0 if not full, -1 for invalid arguments

#### `ssize_t cbuf_get_readable_size(cbuf_t *cbuf)`

Returns the number of bytes available to read from the buffer, or -1 for invalid arguments.

#### `int cbuf_waitfor_readable(cbuf_t *cbuf, size_t nbytes, int64_t timeout_usec)`

Waits until at least nbytes are available to read or timeout occurs.

- Returns >0 when data is available, 0 on timeout, -1 for invalid arguments

### Buffer Data Operations

#### `ssize_t cbuf_write_blocking(cbuf_t *cbuf, const uint8_t *buf, size_t nbytes, int64_t timeout_usec)`

Writes data to the buffer, blocking until space is available or timeout occurs.

- Returns the number of bytes written, or -1 for invalid arguments

#### `ssize_t cbuf_read_blocking(cbuf_t *cbuf, uint8_t *buf, size_t nbytes, int64_t timeout_usec, bool all)`

Reads data from the buffer, blocking until data is available or timeout occurs.

- Set `all` to true to wait for all requested bytes or false to read what's available
- Returns the number of bytes read, or -1 for invalid arguments

#### `ssize_t cbuf_peek(cbuf_t *cbuf, uint8_t *buf, size_t nbytes)`

Reads data from the buffer without consuming it (FIFO ordering).

- Returns the number of bytes read, or -1 for invalid arguments

#### `ssize_t cbuf_remove(cbuf_t *cbuf, size_t nbytes)`

Removes (consumes) data from the buffer without reading it.

- Returns the number of bytes removed, or -1 for invalid arguments

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

This project is licensed under Unlicense.
