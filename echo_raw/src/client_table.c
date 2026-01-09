#include "rawudp.h"
#include <stdlib.h>
#include <string.h>

typedef struct node {
    rawudp_client_key_t key;
    uint32_t counter;
    struct node *next;
} node_t;

struct rawudp_client_table {
    node_t **buckets;
    size_t nb;
};

static uint32_t hash_key(rawudp_client_key_t k) {
    uint32_t ip = (uint32_t)k.ip.s_addr;
    uint32_t p = (uint32_t)k.port;
    uint32_t x = ip ^ (p * 2654435761u);
    x ^= x >> 16;
    x *= 2246822519u;
    x ^= x >> 13;
    x *= 3266489917u;
    x ^= x >> 16;
    return x;
}

static int key_eq(rawudp_client_key_t a, rawudp_client_key_t b) {
    return a.ip.s_addr == b.ip.s_addr && a.port == b.port;
}

rawudp_client_table_t* rawudp_ct_create(void) {
    rawudp_client_table_t *t = (rawudp_client_table_t*)calloc(1, sizeof(*t));
    if (!t) return NULL;
    t->nb = 1024;
    t->buckets = (node_t**)calloc(t->nb, sizeof(node_t*));
    if (!t->buckets) {
        free(t);
        return NULL;
    }
    return t;
}

void rawudp_ct_destroy(rawudp_client_table_t *t) {
    if (!t) return;
    for (size_t i = 0; i < t->nb; i++) {
        node_t *n = t->buckets[i];
        while (n) {
            node_t *nx = n->next;
            free(n);
            n = nx;
        }
    }
    free(t->buckets);
    free(t);
}

int rawudp_ct_next(rawudp_client_table_t *t, rawudp_client_key_t key, uint32_t *out_counter) {
    if (!t || !out_counter) return -1;
    size_t idx = (size_t)(hash_key(key) % (uint32_t)t->nb);
    node_t *n = t->buckets[idx];
    while (n) {
        if (key_eq(n->key, key)) {
            n->counter++;
            *out_counter = n->counter;
            return 0;
        }
        n = n->next;
    }
    n = (node_t*)calloc(1, sizeof(*n));
    if (!n) return -1;
    n->key = key;
    n->counter = 1;
    n->next = t->buckets[idx];
    t->buckets[idx] = n;
    *out_counter = 1;
    return 0;
}

void rawudp_ct_reset(rawudp_client_table_t *t, rawudp_client_key_t key) {
    if (!t) return;
    size_t idx = (size_t)(hash_key(key) % (uint32_t)t->nb);
    node_t *prev = NULL;
    node_t *n = t->buckets[idx];
    while (n) {
        if (key_eq(n->key, key)) {
            if (prev) prev->next = n->next;
            else t->buckets[idx] = n->next;
            free(n);
            return;
        }
        prev = n;
        n = n->next;
    }
}
