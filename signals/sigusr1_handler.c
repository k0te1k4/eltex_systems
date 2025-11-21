#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void sigusr1_handler(int signo)
{
    printf("Получен сигнал %d (SIGUSR1)\n", signo);
}

int main(void)
{
    struct sigaction sa;

    sa.sa_handler = sigusr1_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("PID процесса: %d\n", getpid());
    printf("Жду сигнал SIGUSR1...\n");

    while (1) {
        pause();
    }

    return 0;
}
