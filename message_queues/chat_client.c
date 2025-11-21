#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>

#define SERVER_QUEUE   "/chat_srv"
#define MAX_NAME  32
#define MAX_TEXT  256

struct chat_msg {
    char type;
    char name[MAX_NAME];
    char qname[64];
    char text[MAX_TEXT];
};

static mqd_t server_q = (mqd_t)-1;
static mqd_t client_q = (mqd_t)-1;
static char client_qname[64];
static char nickname[MAX_NAME];

static WINDOW *win_messages;
static WINDOW *win_input;
static pthread_mutex_t screen_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *receiver_thread(void *arg)
{
    struct chat_msg msg;

    (void)arg;

    while (1) {
        ssize_t n = mq_receive(client_q, (char *)&msg,
                               sizeof(msg), NULL);
        if (n == -1) {
            break;
        }

        pthread_mutex_lock(&screen_mutex);
        if (msg.type == 'S') {
            wprintw(win_messages, "[%s]\n", msg.text);
        } else {
            wprintw(win_messages, "%s: %s\n", msg.name, msg.text);
        }
        wrefresh(win_messages);
        pthread_mutex_unlock(&screen_mutex);
    }

    return NULL;
}

int main(void)
{
    struct mq_attr attr;
    pthread_t recv_thread;
    char line[MAX_TEXT];

    printf("Enter your name: ");
    if (fgets(nickname, sizeof(nickname), stdin) == NULL) {
        return 1;
    }
    nickname[strcspn(nickname, "\n")] = '\0';

    snprintf(client_qname, sizeof(client_qname),
             "/chat_%d", getpid());

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct chat_msg);
    attr.mq_curmsgs = 0;

    client_q = mq_open(client_qname, O_CREAT | O_RDONLY, 0666, &attr);
    if (client_q == (mqd_t)-1) {
        perror("mq_open client");
        return 1;
    }

    server_q = mq_open(SERVER_QUEUE, O_WRONLY);
    if (server_q == (mqd_t)-1) {
        perror("mq_open server");
        mq_close(client_q);
        mq_unlink(client_qname);
        return 1;
    }

    struct chat_msg join_msg;
    memset(&join_msg, 0, sizeof(join_msg));
    join_msg.type = 'J';
    strncpy(join_msg.name, nickname, MAX_NAME - 1);
    strncpy(join_msg.qname, client_qname, sizeof(join_msg.qname) - 1);

    if (mq_send(server_q, (const char *)&join_msg,
                sizeof(join_msg), 0) == -1) {
        perror("mq_send join");
        mq_close(client_q);
        mq_unlink(client_qname);
        mq_close(server_q);
        return 1;
    }

    initscr();
    cbreak();
    noecho();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    int msg_h = rows - 3;
    win_messages = newwin(msg_h, cols, 0, 0);
    win_input    = newwin(3, cols, msg_h, 0);

    scrollok(win_messages, TRUE);

    box(win_input, 0, 0);
    mvwprintw(win_input, 1, 1, "> ");
    wrefresh(win_input);
    wrefresh(win_messages);

    if (pthread_create(&recv_thread, NULL,
                       receiver_thread, NULL) != 0) {
        endwin();
        perror("pthread_create");
        mq_close(client_q);
        mq_unlink(client_qname);
        mq_close(server_q);
        return 1;
    }

    while (1) {
        pthread_mutex_lock(&screen_mutex);
        werase(win_input);
        box(win_input, 0, 0);
        mvwprintw(win_input, 1, 1, "> ");
        wrefresh(win_input);

        wgetnstr(win_input, line, sizeof(line) - 1);
        pthread_mutex_unlock(&screen_mutex);

        if (strcmp(line, "/quit") == 0) {
            break;
        }

        struct chat_msg msg;
        memset(&msg, 0, sizeof(msg));
        msg.type = 'M';
        strncpy(msg.name, nickname, MAX_NAME - 1);
        strncpy(msg.text, line, MAX_TEXT - 1);

        if (mq_send(server_q, (const char *)&msg,
                    sizeof(msg), 0) == -1) {
            perror("mq_send msg");
        }
    }

    endwin();

    mq_close(client_q);
    mq_unlink(client_qname);
    mq_close(server_q);

    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);

    return 0;
}
