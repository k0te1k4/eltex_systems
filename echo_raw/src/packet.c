#include "rawudp.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <sys/socket.h>

int rawudp_open_send_socket(void) {
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (s < 0) return -1;
    int on = 1;
    if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        close(s);
        return -1;
    }
    return s;
}

int rawudp_open_recv_socket(void) {
    int s = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    return s;
}

int rawudp_send_packet(int send_sock,
                       const uint8_t *ip_packet, size_t ip_packet_len,
                       struct in_addr dst_ip) {
    struct sockaddr_in dst;
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_addr = dst_ip;

    ssize_t r = sendto(send_sock, ip_packet, ip_packet_len, 0,
                       (struct sockaddr*)&dst, sizeof(dst));
    return (r == (ssize_t)ip_packet_len) ? 0 : -1;
}

int rawudp_build_ipv4_udp(uint8_t *out_buf, size_t out_cap,
                          struct in_addr src_ip, struct in_addr dst_ip,
                          uint16_t src_port, uint16_t dst_port,
                          const uint8_t *payload, size_t payload_len,
                          uint8_t ttl) {
    size_t ip_hl = sizeof(struct iphdr);
    size_t udp_hl = sizeof(struct udphdr);
    size_t total = ip_hl + udp_hl + payload_len;
    if (total > out_cap) return -1;
    if (payload_len > RAWUDP_MAX_PAYLOAD) return -1;

    memset(out_buf, 0, total);

    struct iphdr *ip = (struct iphdr*)out_buf;
    struct udphdr *udp = (struct udphdr*)(out_buf + ip_hl);

    ip->version = 4;
    ip->ihl = (uint8_t)(ip_hl / 4);
    ip->tos = 0;
    ip->tot_len = htons((uint16_t)total);
    ip->id = htons((uint16_t)(getpid() & 0xFFFF));
    ip->frag_off = 0;
    ip->ttl = ttl ? ttl : 64;
    ip->protocol = IPPROTO_UDP;
    ip->saddr = src_ip.s_addr;
    ip->daddr = dst_ip.s_addr;
    ip->check = 0;
    ip->check = rawudp_checksum16(ip, ip_hl);

    udp->source = htons(src_port);
    udp->dest = htons(dst_port);
    udp->len = htons((uint16_t)(udp_hl + payload_len));
    udp->check = 0;

    if (payload_len) {
        memcpy(out_buf + ip_hl + udp_hl, payload, payload_len);
    }

    udp->check = rawudp_udp_checksum(&src_ip, &dst_ip,
                                     (const uint8_t*)udp, udp_hl + payload_len);

    return (int)total;
}

int rawudp_parse_ipv4_udp(const uint8_t *pkt, size_t pkt_len,
                          struct in_addr *src_ip, struct in_addr *dst_ip,
                          uint16_t *src_port, uint16_t *dst_port,
                          const uint8_t **payload, size_t *payload_len) {
    if (pkt_len < sizeof(struct iphdr)) return -1;
    const struct iphdr *ip = (const struct iphdr*)pkt;
    if (ip->version != 4) return -1;
    size_t ip_hl = (size_t)ip->ihl * 4;
    if (ip_hl < sizeof(struct iphdr) || pkt_len < ip_hl + sizeof(struct udphdr)) return -1;
    if (ip->protocol != IPPROTO_UDP) return -1;

    const struct udphdr *udp = (const struct udphdr*)(pkt + ip_hl);
    size_t udp_len = ntohs(udp->len);
    if (udp_len < sizeof(struct udphdr)) return -1;
    if (pkt_len < ip_hl + udp_len) return -1;

    if (src_ip) src_ip->s_addr = ip->saddr;
    if (dst_ip) dst_ip->s_addr = ip->daddr;
    if (src_port) *src_port = ntohs(udp->source);
    if (dst_port) *dst_port = ntohs(udp->dest);

    if (payload) *payload = pkt + ip_hl + sizeof(struct udphdr);
    if (payload_len) *payload_len = udp_len - sizeof(struct udphdr);

    return 0;
}
