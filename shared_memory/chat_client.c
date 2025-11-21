#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ncurses.h>
#include "chat.h"

union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};


static chat_t *g_chat = NULL;
static int     g_semid = -1;
static int     g_running = 1;
static char    g_name[NAME_LEN];

static int     g_msg_row = 1;


static void lock_sem(void)
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op  = -1;
    op.sem_flg = 0;
    semop(g_semid, &op, 1);
}

static void unlock_sem(void)
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op  = 1;
    op.sem_flg = 0;
    semop(g_semid, &op, 1);
}

static void draw_names(void)
{
    int i;
    mvprintw(0, 0, "Users:");
    for (i = 0; i < g_chat->client_count && i < MAX_CLIENTS; i++) {
        mvprintw(i + 1, 0, "%s", g_chat->client_names[i]);
    }
}

static void add_message_line(const char *sender, const char *text)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    if (g_msg_row >= rows - 2) {
        clear();
        g_msg_row = 1;
    }

    mvprintw(g_msg_row, 15, "%s: %s", sender, text);
    g_msg_row++;
}

static void *receiver_thread(void *arg)
{
    int last_seen = 0;

    while (g_running && g_chat->server_running) {
        lock_sem();
        int total = g_chat->message_count;
        while (last_seen < total && last_seen < MAX_MESSAGES) {
            chat_message_t *m = &g_chat->messages[last_seen];
            add_message_line(m->sender, m->text);
            last_seen++;
        }
        draw_names();
        unlock_sem();

        refresh();
        usleep(100000);
    }

    return NULL;
}


static void *sender_thread(void *arg)
{
    int rows, cols;
    char buf[TEXT_LEN];

    while (g_running && g_chat->server_running) {
        getmaxyx(stdscr, rows, cols);
        mvprintw(rows - 1, 0, "You: ");
        clrtoeol();
        move(rows - 1, 5);
        memset(buf, 0, sizeof(buf));
        getnstr(buf, TEXT_LEN - 1);

        if (strcmp(buf, "/quit") == 0) {
            g_running = 0;
            break;
        }

        if (buf[0] == '\0')
            continue;

        lock_sem();
        if (g_chat->message_count < MAX_MESSAGES) {
            chat_message_t *m = &g_chat->messages[g_chat->message_count];
            strncpy(m->sender, g_name, NAME_LEN);
            m->sender[NAME_LEN - 1] = '\0';
            strncpy(m->text, buf, TEXT_LEN);
            m->text[TEXT_LEN - 1] = '\0';
            g_chat->message_count++;
        }
        unlock_sem();
    }

    return NULL;
}

int main(void)
{
    printf("Enter your name: ");
    if (fgets(g_name, sizeof(g_name), stdin) == NULL) {
        return 1;
    }
    size_t len = strlen(g_name);
    if (len > 0 && g_name[len - 1] == '\n') {
        g_name[len - 1] = '\0';
    }

    int shmid = shmget(SHM_KEY, sizeof(chat_t), 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    g_chat = (chat_t *)shmat(shmid, NULL, 0);
    if (g_chat == (void *)-1) {
        perror("shmat");
        return 1;
    }

    g_semid = semget(SEM_KEY, 1, 0666);
    if (g_semid == -1) {
        perror("semget");
        return 1;
    }

    lock_sem();
    if (g_chat->client_count < MAX_CLIENTS) {
        strncpy(g_chat->client_names[g_chat->client_count], g_name, NAME_LEN);
        g_chat->client_names[g_chat->client_count][NAME_LEN - 1] = '\0';
        g_chat->client_count++;

        if (g_chat->message_count < MAX_MESSAGES) {
            chat_message_t *m = &g_chat->messages[g_chat->message_count];
            strncpy(m->sender, "server", NAME_LEN);
            m->sender[NAME_LEN - 1] = '\0';
            snprintf(m->text, TEXT_LEN, "%s joined", g_name);
            g_chat->message_count++;
        }
    }
    unlock_sem();

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    clear();

    pthread_t recv_thr, send_thr;
    pthread_create(&recv_thr, NULL, receiver_thread, NULL);
    pthread_create(&send_thr, NULL, sender_thread, NULL);

    pthread_join(send_thr, NULL);
    g_running = 0;
    pthread_join(recv_thr, NULL);

    endwin();

    if (shmdt(g_chat) == -1) {
        perror("shmdt");
    }

    return 0;
}
