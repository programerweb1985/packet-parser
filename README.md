# packet-parser

A high-performance, allocation-free network packet parser written in a
traditional C style within C++. It reads raw binary data directly from a
socket receive buffer with zero intermediate copies.

## Overview

- Extracts a 2-byte `packet_length` header (little-endian) from the buffer.
- Copies the payload into a fixed-size local buffer using pointer arithmetic,
  keeping the receive hot path free of library-call overhead.
- Handles partial frames by rewinding the read offset so the caller can
  `recv()` more data and retry.

## Design

The parser is intentionally minimal and allocation-free so it can live inside
a latency-sensitive receive loop. State is held in a small `Parser` struct
(`buf`, `buf_len`, `pos`), and the parse function performs only pointer
math and cheap integer comparisons.

## Build

```bash
make          # build the server demo (Linux)
make asan     # build the sanitizer test build
```

## Sanitizer verification

All memory access is validated with AddressSanitizer + UndefinedBehaviorSanitizer:

```bash
make asan
./exploit_asan
```

## Layout

| File         | Purpose                                   |
|--------------|-------------------------------------------|
| `parser.h`   | Public interface                          |
| `parser.cpp` | Parser implementation                     |
| `main.cpp`   | Socket receive-loop demo (Linux)          |
| `exploit.cpp`| Test harness (used by the ASan build)     |

## License

MIT
