#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
    int flag;
    char text[64];
} shm_data_t;

int main(void)
{
    key_t key = 0x1234;
    int shmid = shmget(key, sizeof(shm_data_t), 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    shm_data_t *data = (shm_data_t *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat");
        return 1;
    }

    printf("Client: waiting for server message...\n");
    while (data->flag != 1) {
        usleep(100000);
    }

    printf("Client: got: %s\n", data->text);

    strncpy(data->text, "Hello!", sizeof(data->text));
    data->text[sizeof(data->text) - 1] = '\0';
    data->flag = 2;

    printf("Client: sent reply: %s\n", data->text);

    if (shmdt(data) == -1) {
        perror("shmdt");
    }

    return 0;
}
