// client.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    const char *fifo = "myfifo";
    char buf[64];

    int fd = open(fifo, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    read(fd, buf, sizeof(buf));
    printf("Client read: %s\n", buf);
    close(fd);

    unlink(fifo); // удаляем канал
    return 0;
}

