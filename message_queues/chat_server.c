#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#define SERVER_QUEUE   "/chat_srv"
#define MAX_NAME  32
#define MAX_TEXT  256
#define MAX_CLIENTS 10
#define MAX_HISTORY 100

struct chat_msg {
    char type;             // 'J' - join, 'M' - msg, 'S' - system
    char name[MAX_NAME];
    char qname[64];
    char text[MAX_TEXT];
};

struct client_info {
    char name[MAX_NAME];
    char qname[64];
    mqd_t q;
};

static struct client_info clients[MAX_CLIENTS];
static int client_count = 0;

static struct chat_msg history[MAX_HISTORY];
static int history_count = 0;

static mqd_t server_q = (mqd_t)-1;

static void send_to_all(const struct chat_msg *msg)
{
    int i;
    for (i = 0; i < client_count; i++) {
        if (clients[i].q == (mqd_t)-1)
            continue;
        if (mq_send(clients[i].q, (const char *)msg,
                    sizeof(*msg), 0) == -1) {
            perror("mq_send (to client)");
        }
    }
}

static void handle_join(struct chat_msg *msg)
{
    if (client_count >= MAX_CLIENTS) {
        struct chat_msg sys;
        memset(&sys, 0, sizeof(sys));
        sys.type = 'S';
        strcpy(sys.name, "Server");
        snprintf(sys.text, MAX_TEXT,
                 "Too many clients, %s rejected", msg->name);

        mqd_t tmp = mq_open(msg->qname, O_WRONLY);
        if (tmp != (mqd_t)-1) {
            mq_send(tmp, (const char *)&sys, sizeof(sys), 0);
            mq_close(tmp);
        }
        return;
    }

    mqd_t cq = mq_open(msg->qname, O_WRONLY);
    if (cq == (mqd_t)-1) {
        perror("mq_open client");
        return;
    }

    struct client_info *cl = &clients[client_count++];
    strcpy(cl->name, msg->name);
    strcpy(cl->qname, msg->qname);
    cl->q = cq;

    int i;
    for (i = 0; i < history_count; i++) {
        if (mq_send(cl->q, (const char *)&history[i],
                    sizeof(history[i]), 0) == -1) {
            perror("mq_send history");
        }
    }

    struct chat_msg join_msg;
    memset(&join_msg, 0, sizeof(join_msg));
    join_msg.type = 'S';
    strcpy(join_msg.name, "Server");
    snprintf(join_msg.text, MAX_TEXT, "%s joined chat", msg->name);

    if (history_count < MAX_HISTORY) {
        history[history_count++] = join_msg;
    }

    send_to_all(&join_msg);
}

static void handle_message(struct chat_msg *msg)
{
    if (history_count < MAX_HISTORY) {
        history[history_count++] = *msg;
    }
    send_to_all(msg);
}

static void cleanup(int sig)
{
    int i;
    if (server_q != (mqd_t)-1) {
        mq_close(server_q);
        mq_unlink(SERVER_QUEUE);
    }

    for (i = 0; i < client_count; i++) {
        if (clients[i].q != (mqd_t)-1) {
            mq_close(clients[i].q);
        }
    }

    printf("Server: exit\n");
    _exit(0);
}

int main(void)
{
    struct mq_attr attr;
    struct chat_msg msg;

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    mq_unlink(SERVER_QUEUE);

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct chat_msg);
    attr.mq_curmsgs = 0;

    server_q = mq_open(SERVER_QUEUE, O_CREAT | O_RDONLY, 0666, &attr);
    if (server_q == (mqd_t)-1) {
        perror("mq_open server");
        return 1;
    }

    printf("Server: started, waiting for clients...\n");

    while (1) {
        ssize_t n = mq_receive(server_q, (char *)&msg,
                               sizeof(msg), NULL);
        if (n == -1) {
            perror("mq_receive");
            continue;
        }

        if (msg.type == 'J') {
            handle_join(&msg);
        } else if (msg.type == 'M') {
            handle_message(&msg);
        }
    }

    return 0;
}
