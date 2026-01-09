#pragma once

#include <stdint.h>
#include <stddef.h>
#include <netinet/in.h>

#define RAWUDP_MAX_PAYLOAD 1400

uint16_t rawudp_checksum16(const void *data, size_t len);

uint16_t rawudp_udp_checksum(const struct in_addr *src, const struct in_addr *dst,
                             const uint8_t *udp_hdr_and_data, size_t udp_len);

int rawudp_build_ipv4_udp(uint8_t *out_buf, size_t out_cap,
                          struct in_addr src_ip, struct in_addr dst_ip,
                          uint16_t src_port, uint16_t dst_port,
                          const uint8_t *payload, size_t payload_len,
                          uint8_t ttl);

int rawudp_parse_ipv4_udp(const uint8_t *pkt, size_t pkt_len,
                          struct in_addr *src_ip, struct in_addr *dst_ip,
                          uint16_t *src_port, uint16_t *dst_port,
                          const uint8_t **payload, size_t *payload_len);

int rawudp_open_send_socket(void);
int rawudp_open_recv_socket(void);

int rawudp_send_packet(int send_sock,
                       const uint8_t *ip_packet, size_t ip_packet_len,
                       struct in_addr dst_ip);

typedef struct {
    struct in_addr ip;
    uint16_t port; // host order
} rawudp_client_key_t;

typedef struct rawudp_client_table rawudp_client_table_t;

rawudp_client_table_t* rawudp_ct_create(void);
void rawudp_ct_destroy(rawudp_client_table_t *t);

int rawudp_ct_next(rawudp_client_table_t *t, rawudp_client_key_t key, uint32_t *out_counter);
void rawudp_ct_reset(rawudp_client_table_t *t, rawudp_client_key_t key);
