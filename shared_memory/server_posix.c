#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct {
    int flag;
    char text[64];
} shm_data_t;

#define SHM_NAME "/my_posix_shm"

int main(void)
{
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(fd, sizeof(shm_data_t)) == -1) {
        perror("ftruncate");
        return 1;
    }

    shm_data_t *data = (shm_data_t *)mmap(NULL, sizeof(shm_data_t),
                                          PROT_READ | PROT_WRITE,
                                          MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    close(fd);

    data->flag = 0;
    strncpy(data->text, "Hi!", sizeof(data->text));
    data->text[sizeof(data->text) - 1] = '\0';
    data->flag = 1;

    printf("POSIX server: sent: %s\n", data->text);
    printf("POSIX server: waiting for client...\n");

    while (data->flag != 2) {
        usleep(100000);
    }

    printf("POSIX server: got reply: %s\n", data->text);

    if (munmap(data, sizeof(shm_data_t)) == -1) {
        perror("munmap");
    }

    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink");
    }

    return 0;
}
