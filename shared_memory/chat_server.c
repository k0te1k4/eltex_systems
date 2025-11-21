#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "chat.h"


union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

int main(void)
{
    int shmid = shmget(SHM_KEY, sizeof(chat_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    chat_t *chat = (chat_t *)shmat(shmid, NULL, 0);
    if (chat == (void *)-1) {
        perror("shmat");
        return 1;
    }

    memset(chat, 0, sizeof(chat_t));
    chat->server_running = 1;

    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl");
        return 1;
    }

    printf("Chat server started.\n");
    printf("Press 'q' then Enter to stop server.\n");

    int c;
    while ((c = getchar()) != EOF) {
        if (c == 'q')
            break;
    }

    chat->server_running = 0;

    if (shmdt(chat) == -1) {
        perror("shmdt");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
    }

    printf("Server stopped.\n");
    return 0;
}
