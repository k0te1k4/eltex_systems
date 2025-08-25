#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256
#define MAX_ARGS 64

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    pid_t pid;
    int status;

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) break;

        // Убираем \n
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, "exit") == 0) {
            break;
        }

        // Разбиваем строку на аргументы
        int argc = 0;
        char *token = strtok(line, " ");
        while (token && argc < MAX_ARGS-1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;

        if (argc == 0) continue;

        pid = fork();
        if (pid == 0) {
            // Дочерний процесс
            execvp(args[0], args);
            perror("exec failed");
            exit(1);
        } else if (pid > 0) {
            // Родитель ждёт
            waitpid(pid, &status, 0);
        } else {
            perror("fork error");
        }
    }
    return 0;
}

