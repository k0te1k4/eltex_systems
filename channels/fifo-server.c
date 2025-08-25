// server.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    const char *fifo = "myfifo";
    const char *msg = "Hi!";
    
    // создаём FIFO
    if (mkfifo(fifo, 0666) == -1) {
        perror("mkfifo");
    }

    int fd = open(fifo, O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    write(fd, msg, strlen(msg) + 1);
    close(fd);
    return 0;
}

