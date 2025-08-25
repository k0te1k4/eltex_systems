#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid < 0) {
        perror("fork error");
        exit(1);
    }

    if (pid == 0) {  
        // Дочерний процесс
        printf("Child: PID=%d, PPID=%d\n", getpid(), getppid());
        exit(42); // произвольный код выхода
    } else {      
        // Родительский процесс
        printf("Parent: PID=%d, PPID=%d\n", getpid(), getppid());
        wpid = wait(&status); 
        if (WIFEXITED(status)) {
            printf("Parent: child %d exited with status %d\n",
                   wpid, WEXITSTATUS(status));
        }
    }
    return 0;
}

