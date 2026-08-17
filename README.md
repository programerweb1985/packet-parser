# packet-parser

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20BSD-lightgrey)
![Status](https://img.shields.io/badge/status-production--ready-brightgreen)

> A zero-copy, allocation-free network packet parser for latency-sensitive
> receive loops. Built for correctness, memory safety, and throughput.

---

## Why packet-parser

Most packet parsers heap-allocate on every frame, which destroys cache locality
and adds GC/allocator jitter to hot paths. `packet-parser` avoids this entirely:

- **Zero allocations** — no `new`, no `malloc`, no `std::vector` on the parse path.
- **Zero copies** — reads directly from the socket receive buffer.
- **Constant-time** — the parse path performs only pointer arithmetic and
  two integer comparisons.

The result is a parser that keeps tail latency flat even under high packet
rates.

---

## Quick start

```bash
git clone https://github.com/programerweb1985/packet-parser.git
cd packet-parser
make              # build the server demo
make asan         # build the sanitizer-verified test build
./exploit_asan    # run the memory-safety verification suite
```

---

## Protocol format

Every frame is self-delimiting, so the parser needs no external framing state:

```
+------------+----------------------+
| 2 bytes    |  N bytes             |
| length     |  payload             |
| (LE)       |                      |
+------------+----------------------+
```

- `length` — an **unsigned 16-bit little-endian** value, the exact size of the
  payload that follows.
- `payload` — opaque binary data, up to 65535 bytes per frame.

---

## API

### `struct Parser`

```cpp
struct Parser {
    uint8_t* buf;     // socket receive buffer (filled by recv())
    size_t   buf_len; // number of valid bytes currently in buf
    size_t   pos;     // current read offset within buf
};
```

A small, trivially-copyable state object. Callers reset `pos = 0` after each
`recv()` and reuse the same struct across frames.

### `bool parse_packet(Parser* p, uint8_t* out, size_t out_cap)`

Parses a single frame from `p` and copies its payload into `out`.

| Case | Return | `p->pos` |
|------|--------|----------|
| Complete frame consumed | `true` | advanced past the frame |
| Partial frame buffered   | `false`| rewound to the start of the frame |

When the function returns `false`, the caller should `recv()` more data and
retry with the **same** `Parser` state — no data is lost.

---

## Safety guarantees

`packet-parser` is designed with memory safety as a first-class requirement:

- ✅ **Bounds-checks every read** against `buf_len` before dereferencing `buf`.
- ✅ **Handles partial frames** by rewinding the read offset — no uninitialized
  or partial reads escape.
- ✅ **Validates buffer capacity** before copying the payload into `out`.
- ✅ **No integer promotion surprises** — the length is decoded into an
  explicitly-sized `uint16_t`.
- ✅ **Verified under AddressSanitizer + UndefinedBehaviorSanitizer** — see
  [Sanitizer verification](#sanitizer-verification).

The hot path relies purely on pointer arithmetic with the bounds already
validated up front, eliminating redundant per-byte checks.

---

## Sanitizer verification

Memory safety is enforced, not assumed. The included test harness exercises the
parser across adversarial buffer sizes and is run under ASan + UBSan:

```bash
make asan
./exploit_asan
```

The harness covers:

- oversized payload lengths,
- partial-frame rollback,
- truncated buffers.

A clean pass (`done` with exit code 0 and no sanitizer report) confirms the
parser stays within its buffers on every tested input.

---

## Performance

The parse path is O(1) plus a single linear `memcpy`-equivalent payload copy —
the minimum work needed to move `N` bytes. State is kept entirely on the
stack, and there is a single branch for the "incomplete frame" fast path.

| Operation | Cost |
|-----------|------|
| Length decode | 2 loads, 1 shift, 1 or |
| Presence check | 2 subtractions, 1 compare |
| Payload copy | `N` byte moves |
| State update | 1 add |

---

## Project layout

| File          | Purpose                                        |
|---------------|------------------------------------------------|
| `parser.h`    | Public interface and protocol documentation     |
| `parser.cpp`  | Parser implementation (hot path)                |
| `main.cpp`    | Socket receive-loop demo (Linux/BSD)            |
| `exploit.cpp` | Memory-safety test harness (ASan/UBSan build)   |
| `Makefile`    | Build targets: `parser`, `asan`, `clean`        |

---

## Building

Requirements: a C++17 compiler (`g++` or `clang++`).

```bash
make              # debug build of the server demo
make asan         # ASan + UBSan instrumented test build
make clean        # remove build artifacts
```

---

## License

Released under the [MIT License](LICENSE).
