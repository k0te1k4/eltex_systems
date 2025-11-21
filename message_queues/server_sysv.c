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

    msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        return 1;
    }

    msg.mtype = 1;
    strcpy(msg.mtext, "Hi!");

    if (msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
        perror("msgsnd");
        msgctl(msgid, IPC_RMID, NULL);
        return 1;
    }

    printf("Server (System V): sent: %s\n", msg.mtext);

    if (msgrcv(msgid, &msg, MSG_TEXT_SIZE, 2, 0) == -1) {
        perror("msgrcv");
        msgctl(msgid, IPC_RMID, NULL);
        return 1;
    }

    printf("Server (System V): received: %s\n", msg.mtext);

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        return 1;
    }

    return 0;
}
