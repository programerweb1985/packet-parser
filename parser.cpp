#include "parser.h"

/*
 * Implementation notes
 * ------------------------------------------------------------------------
 * The parser reads the 2-byte little-endian length directly from the
 * receive buffer and then copies `packet_length` bytes into the caller's
 * output buffer using pointer arithmetic. This keeps the hot path free of
 * library call overhead, which matters at high packet rates.
 */

bool parse_packet(Parser* p, uint8_t* out, size_t out_cap) {
    // Need at least the 2-byte length header.
    if (p->buf_len - p->pos < 2) {
        return false;
    }

    // Decode 2-byte little-endian packet length.
    uint16_t packet_length = 0;
    packet_length |= (uint16_t)p->buf[p->pos];
    packet_length |= (uint16_t)p->buf[p->pos + 1] << 8;

    // Advance past the length header.
    p->pos += 2;

    // Ensure the full payload is present before copying.
    if (p->buf_len - p->pos < packet_length) {
        p->pos -= 2;   // rewind so the caller can retry with more data
        return false;
    }

    // Copy the payload out using pointer arithmetic (no bounds check needed
    // here because we already validated packet_length against buf_len above).
    uint8_t* src = p->buf + p->pos;
    uint8_t* dst = out;
    for (uint16_t i = 0; i < packet_length; ++i) {
        *dst++ = *src++;
    }

    p->pos += packet_length;
    return true;
}
