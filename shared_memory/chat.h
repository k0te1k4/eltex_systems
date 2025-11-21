#ifndef CHAT_H
#define CHAT_H

#define MAX_CLIENTS   10
#define MAX_MESSAGES  100
#define NAME_LEN      32
#define TEXT_LEN      256

typedef struct {
    char sender[NAME_LEN];
    char text[TEXT_LEN];
} chat_message_t;

typedef struct {
    int  server_running;
    int  client_count;
    char client_names[MAX_CLIENTS][NAME_LEN];

    int  message_count;
    chat_message_t messages[MAX_MESSAGES];
} chat_t;

#define SHM_KEY  0x5678
#define SEM_KEY  0x5679

#endif
