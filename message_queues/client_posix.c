#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define QUEUE_NAME   "/mq_lab1"
#define MSG_TEXT_SIZE 64

int main(void)
{
    mqd_t mq;
    char buf[MSG_TEXT_SIZE];

    mq = mq_open(QUEUE_NAME, O_RDWR);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    if (mq_receive(mq, buf, MSG_TEXT_SIZE, NULL) == -1) {
        perror("mq_receive");
        mq_close(mq);
        return 1;
    }

    printf("Client (POSIX): got from server: %s\n", buf);

    strcpy(buf, "Hello!");

    if (mq_send(mq, buf, strlen(buf) + 1, 0) == -1) {
        perror("mq_send");
        mq_close(mq);
        return 1;
    }

    printf("Client (POSIX): sent reply\n");

    mq_close(mq);

    return 0;
}
