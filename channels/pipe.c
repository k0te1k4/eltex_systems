#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid;
    char buf[64];
    const char *msg = "Hi!";

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {  // дочерний
        close(fd[1]); // закрываем запись
        read(fd[0], buf, sizeof(buf));
        printf("Child read: %s\n", buf);
        close(fd[0]);
        exit(0);
    } else {         // родитель
        close(fd[0]); // закрываем чтение
        write(fd[1], msg, strlen(msg) + 1);
        close(fd[1]);
        wait(NULL);
    }
    return 0;
}

