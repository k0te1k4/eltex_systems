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
    struct mq_attr attr;
    char buf[MSG_TEXT_SIZE];

    mq_unlink(QUEUE_NAME);

    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_TEXT_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    strcpy(buf, "Hi!");

    if (mq_send(mq, buf, strlen(buf) + 1, 0) == -1) {
        perror("mq_send");
        mq_close(mq);
        mq_unlink(QUEUE_NAME);
        return 1;
    }

    printf("Server (POSIX): sent: %s\n", buf);

    if (mq_receive(mq, buf, MSG_TEXT_SIZE, NULL) == -1) {
        perror("mq_receive");
        mq_close(mq);
        mq_unlink(QUEUE_NAME);
        return 1;
    }

    printf("Server (POSIX): received: %s\n", buf);

    mq_close(mq);
    mq_unlink(QUEUE_NAME);

    return 0;
}
