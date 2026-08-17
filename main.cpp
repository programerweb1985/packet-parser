#include "parser.h"
#include <cstdio>
#include <cstdint>

// Minimal receive-loop demo (Linux/BSD sockets).
// Binds a TCP socket and parses incoming frames.
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(fd, 1);

    int c = accept(fd, nullptr, nullptr);

    uint8_t buf[4096];     // receive buffer
    uint8_t payload[48];   // per-frame payload target
    Parser p{};

    for (;;) {
        ssize_t n = recv(c, buf, sizeof(buf), 0);
        if (n <= 0) break;

        p.buf = buf;
        p.buf_len = (size_t)n;
        p.pos = 0;

        // Parse every complete frame currently buffered.
        while (parse_packet(&p, payload, sizeof(payload))) {
            // process(payload) ...
            (void)payload;
        }
    }

    close(c);
    close(fd);
    return 0;
}
#else
int main() {
    std::printf("socket demo is Linux-only\n");
    return 0;
}
#endif
