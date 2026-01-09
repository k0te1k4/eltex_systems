#include "rawudp.h"
#include <string.h>
#include <arpa/inet.h>

uint16_t rawudp_checksum16(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t*)data;
    uint32_t sum = 0;

    while (len > 1) {
        sum += (uint16_t)((p[0] << 8) | p[1]);
        p += 2;
        len -= 2;
    }
    if (len == 1) {
        sum += (uint16_t)(p[0] << 8);
    }

    while (sum >> 16) sum = (sum & 0xFFFFu) + (sum >> 16);
    return (uint16_t)(~sum);
}

uint16_t rawudp_udp_checksum(const struct in_addr *src, const struct in_addr *dst,
                             const uint8_t *udp_hdr_and_data, size_t udp_len) {
    struct pseudo {
        uint32_t saddr;
        uint32_t daddr;
        uint8_t  zero;
        uint8_t  proto;
        uint16_t len;
    } ph;

    ph.saddr = src->s_addr;
    ph.daddr = dst->s_addr;
    ph.zero = 0;
    ph.proto = IPPROTO_UDP;
    ph.len = htons((uint16_t)udp_len);

    uint32_t sum = 0;

    uint16_t c1 = rawudp_checksum16(&ph, sizeof(ph));
    sum += (uint16_t)(~c1);

    const uint8_t *p = udp_hdr_and_data;
    size_t len = udp_len;

    while (len > 1) {
        sum += (uint16_t)((p[0] << 8) | p[1]);
        p += 2;
        len -= 2;
    }
    if (len == 1) {
        sum += (uint16_t)(p[0] << 8);
    }

    while (sum >> 16) sum = (sum & 0xFFFFu) + (sum >> 16);

    uint16_t res = (uint16_t)(~sum);
    if (res == 0) res = 0xFFFF;
    return res;
}
