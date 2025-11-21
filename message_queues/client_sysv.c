#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MSG_TEXT_SIZE 64

struct msgbuf {
    long mtype;
    char mtext[MSG_TEXT_SIZE];
};

int main(void)
{
    key_t key;
    int msgid;
    struct msgbuf msg;

    key = ftok(".", 'Q');
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    msgid = msgget(key, 0);
    if (msgid == -1) {
        perror("msgget");
        return 1;
    }

    if (msgrcv(msgid, &msg, MSG_TEXT_SIZE, 1, 0) == -1) {
        perror("msgrcv");
        return 1;
    }

    printf("Client (System V): got from server: %s\n", msg.mtext);

    msg.mtype = 2;
    strcpy(msg.mtext, "Hello!");

    if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
        perror("msgsnd");
        return 1;
    }

    printf("Client (System V): sent reply\n");

    return 0;
}
