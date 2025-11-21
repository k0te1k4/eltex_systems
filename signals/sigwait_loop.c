#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(void)
{
    sigset_t set;
    int sig;
    int res;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);


    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("PID процесса: %d\n", getpid());
    printf("SIGUSR1 заблокирован. Жду сигнал через sigwait()...\n");

    while (1) {
        res = sigwait(&set, &sig);
        if (res != 0) {
            printf("Ошибка sigwait: %d\n", res);
            break;
        }

        printf("sigwait() вернул сигнал %d (ожидали SIGUSR1)\n", sig);
    }

    return 0;
}
