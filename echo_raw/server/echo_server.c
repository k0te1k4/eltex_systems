#include "rawudp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

static void die(const char *m) {
    perror(m);
    exit(1);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <listen_port>\n", argv[0]);
        return 1;
    }

    uint16_t listen_port = (uint16_t)atoi(argv[1]);
    if (!listen_port) {
        fprintf(stderr, "bad port\n");
        return 1;
    }

    int rs = rawudp_open_recv_socket();
    if (rs < 0) die("recv socket");

    int ss = rawudp_open_send_socket();
    if (ss < 0) die("send socket");

    rawudp_client_table_t *ct = rawudp_ct_create();
    if (!ct) die("client table");

    uint8_t buf[65536];
    uint8_t out[65536];

    for (;;) {
        ssize_t n = recv(rs, buf, sizeof(buf), 0);
        if (n <= 0) continue;

        struct in_addr sip, dip;
        uint16_t sport, dport;
        const uint8_t *payload;
        size_t plen;

        if (rawudp_parse_ipv4_udp(buf, (size_t)n, &sip, &dip, &sport, &dport, &payload, &plen) != 0)
            continue;

        if (dport != listen_port) continue;

        rawudp_client_key_t key;
        key.ip = sip;
        key.port = sport;

        if (plen == 8 && memcmp(payload, "shutdown", 8) == 0) {
            rawudp_ct_reset(ct, key);
            continue;
        }

        uint32_t cnt;
        if (rawudp_ct_next(ct, key, &cnt) != 0) continue;

        char msg[RAWUDP_MAX_PAYLOAD + 64];
        size_t copy = plen;
        if (copy > RAWUDP_MAX_PAYLOAD) copy = RAWUDP_MAX_PAYLOAD;
        memcpy(msg, payload, copy);
        msg[copy] = 0;

        char resp[RAWUDP_MAX_PAYLOAD + 128];
        int rn = snprintf(resp, sizeof(resp), "%s %u", msg, cnt);
        if (rn < 0) continue;
        size_t rlen = (size_t)rn;

        int pkt_len = rawudp_build_ipv4_udp(out, sizeof(out),
                                            dip, sip,
                                            listen_port, sport,
                                            (const uint8_t*)resp, rlen,
                                            64);
        if (pkt_len < 0) continue;

        rawudp_send_packet(ss, out, (size_t)pkt_len, sip);
    }

    rawudp_ct_destroy(ct);
    close(ss);
    close(rs);
    return 0;
}
