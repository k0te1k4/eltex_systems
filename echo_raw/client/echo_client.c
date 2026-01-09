#include "rawudp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

static void die(const char *m) {
    perror(m);
    exit(1);
}

static uint16_t pick_port(void) {
    srand((unsigned)time(NULL) ^ (unsigned)getpid());
    return (uint16_t)(20000 + (rand() % 40000));
}

int main(int argc, char **argv) {
    if (argc != 3 && argc != 4) {
        fprintf(stderr, "usage: %s <server_ip> <server_port> [local_port]\n", argv[0]);
        return 1;
    }

    struct in_addr server_ip;
    if (inet_aton(argv[1], &server_ip) == 0) {
        fprintf(stderr, "bad server ip\n");
        return 1;
    }

    uint16_t server_port = (uint16_t)atoi(argv[2]);
    if (!server_port) {
        fprintf(stderr, "bad server port\n");
        return 1;
    }

    uint16_t local_port = (argc == 4) ? (uint16_t)atoi(argv[3]) : pick_port();
    if (!local_port) {
        fprintf(stderr, "bad local port\n");
        return 1;
    }

    int rs = rawudp_open_recv_socket();
    if (rs < 0) die("recv socket");

    int ss = rawudp_open_send_socket();
    if (ss < 0) die("send socket");

    uint8_t out[65536];
    uint8_t inbuf[65536];

    struct in_addr src_ip;
    src_ip.s_addr = INADDR_ANY;

    char line[RAWUDP_MAX_PAYLOAD];

    for (;;) {
        if (!fgets(line, sizeof(line), stdin)) break;
        size_t len = strlen(line);
        while (len && (line[len-1] == '\n' || line[len-1] == '\r')) line[--len] = 0;

        int pkt_len = rawudp_build_ipv4_udp(out, sizeof(out),
                                            src_ip, server_ip,
                                            local_port, server_port,
                                            (const uint8_t*)line, len,
                                            64);
        if (pkt_len < 0) continue;

        if (rawudp_send_packet(ss, out, (size_t)pkt_len, server_ip) != 0) {
            perror("send");
            continue;
        }

        if (strcmp(line, "shutdown") == 0) break;

        for (;;) {
            ssize_t n = recv(rs, inbuf, sizeof(inbuf), 0);
            if (n <= 0) continue;

            struct in_addr sip, dip;
            uint16_t sport, dport;
            const uint8_t *payload;
            size_t plen;

            if (rawudp_parse_ipv4_udp(inbuf, (size_t)n, &sip, &dip, &sport, &dport, &payload, &plen) != 0)
                continue;

            if (sip.s_addr != server_ip.s_addr) continue;
            if (sport != server_port) continue;
            if (dport != local_port) continue;

            fwrite(payload, 1, plen, stdout);
            fputc('\n', stdout);
            fflush(stdout);
            break;
        }
    }

    close(ss);
    close(rs);
    return 0;
}
