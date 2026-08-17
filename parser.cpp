#include "parser.h"

/*
 * Implementation notes
 * ------------------------------------------------------------------------
 * The parser reads the 2-byte little-endian length directly from the
 * receive buffer and then copies `packet_length` bytes into the caller's
 * output buffer using pointer arithmetic. This keeps the hot path free of
 * library call overhead, which matters at high packet rates.
 *
 * Hardening (see GHSA-q2rv-qfh4-8rv2):
 *  - NULL-pointer validation on all pointer arguments.
 *  - Defensive guard against pos > buf_len (integer underflow).
 *  - Safe rollback of the read offset (no unsigned wrap).
 *  - Output capacity (out_cap) validated before copying the payload.
 */

bool parse_packet(Parser* p, uint8_t* out, size_t out_cap) {
    if (!p || !p->buf || !out) return false;       // CWE-476
    if (p->pos > p->buf_len) return false;         // CWE-191

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
        if (p->pos >= 2) p->pos -= 2;              // safe rollback (CWE-191)
        return false;
    }

    // Ensure the destination buffer can hold the payload (critical fix).
    if (packet_length > out_cap) {                 // CWE-787
        return false;
    }

    // Copy the payload out using pointer arithmetic. Bounds are now fully
    // validated for both the source and destination buffers.
    uint8_t* src = p->buf + p->pos;
    uint8_t* dst = out;
    for (uint16_t i = 0; i < packet_length; ++i) {
        *dst++ = *src++;
    }

    p->pos += packet_length;
    return true;
}
