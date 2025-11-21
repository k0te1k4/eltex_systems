#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Использование: %s <pid> <USR1|INT>\n", argv[0]);
        return 1;
    }

    pid_t pid = (pid_t)atoi(argv[1]);
    int sig;

    if (strcmp(argv[2], "USR1") == 0) {
        sig = SIGUSR1;
    } else if (strcmp(argv[2], "INT") == 0) {
        sig = SIGINT;
    } else {
        fprintf(stderr, "Неизвестный сигнал: %s\nИспользуйте USR1 или INT\n", argv[2]);
        return 1;
    }

    if (kill(pid, sig) == -1) {
        perror("kill");
        return 1;
    }

    printf("Сигнал отправлен\n");
    return 0;
}
