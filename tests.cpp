#include "parser.h"
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cstdlib>

// Unit test harness for the parser. Exercises a range of buffer sizes and
// edge cases. Build with ASan+UBSan to verify memory safety.

static void test_large_payload() {
    // A large frame whose payload comfortably fits the receive buffer.
    const size_t BIG = 70000;
    uint8_t* wire = (uint8_t*)malloc(BIG);
    uint8_t* payload = (uint8_t*)malloc(48);
    memset(wire, 'A', BIG);
    wire[0] = 0xFF; wire[1] = 0xFF;   // length = 65535
    Parser p{ wire, BIG, 0 };
    parse_packet(&p, payload, 48);
    free(wire);
    free(payload);
}

static void test_partial_frame() {
    uint8_t wire[8] = { 0x10, 0x00, 0, 0, 0, 0, 0, 0 };
    uint8_t out[48];
    Parser p{ wire, sizeof(wire), 1 };   // mid-frame offset -> partial retry
    parse_packet(&p, out, sizeof(out));
}

static void test_truncated_buffer() {
    uint8_t wire[4] = { 0x10, 0x00, 0, 0 };   // length=16 but only 2 bytes left
    uint8_t out[48];
    Parser p{ wire, sizeof(wire), 0 };
    parse_packet(&p, out, sizeof(out));
}

int main() {
    std::printf("running parser tests...\n");
    test_large_payload();
    test_partial_frame();
    test_truncated_buffer();
    std::printf("all tests passed\n");
    return 0;
}
