#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void print_info(const char *name) {
    printf("%s: PID=%d, PPID=%d\n", name, getpid(), getppid());
}

int main() {
    pid_t pid1, pid2;

    // Процесс1
    if ((pid1 = fork()) == 0) {
        print_info("Process1");

        pid_t pid3 = fork();
        if (pid3 == 0) {
            print_info("Process3");
            exit(0);
        }

        pid_t pid4 = fork();
        if (pid4 == 0) {
            print_info("Process4");
            exit(0);
        }

        // Ждем 3 и 4
        wait(NULL);
        wait(NULL);
        exit(0);
    }

    // Процесс2
    if ((pid2 = fork()) == 0) {
        print_info("Process2");

        pid_t pid5 = fork();
        if (pid5 == 0) {
            print_info("Process5");
            exit(0);
        }

        wait(NULL); 
        exit(0);
    }

    // Родитель
    print_info("Parent");
    wait(NULL); // ждем Process1
    wait(NULL); // ждем Process2
    return 0;
}

