#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(void)
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("PID процесса: %d\n", getpid());
    printf("SIGINT заблокирован. Попробуйте нажать Ctrl+C или послать SIGINT.\n");

    while (1) {
        sleep(1);
    }

    return 0;
}
