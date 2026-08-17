#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include <cstdint>
#include <cstddef>
#include <cstring>

/*
 * High-performance network packet parser.
 *
 * Designed for zero-copy, minimal-overhead parsing of raw socket buffers.
 * The 2-byte packet_length prefix is read directly at the buffer offset,
 * followed by the payload which is copied out via pointer arithmetic.
 *
 * This interface is intentionally small and allocation-free so it can be
 * used in hot, latency-sensitive receive loops.
 */

struct Parser {
    uint8_t* buf;        // socket receive buffer (filled by recv())
    size_t   buf_len;    // number of valid bytes currently in buf
    size_t   pos;        // current read offset within buf
};

/*
 * Parse a single frame from `p` and copy its payload into `out`.
 *
 * Returns true when a complete frame was consumed and copied;
 * returns false when the buffer holds a partial frame (caller should
 * recv() more data and retry).
 */
bool parse_packet(Parser* p, uint8_t* out, size_t out_cap);

#endif // PACKET_PARSER_H
