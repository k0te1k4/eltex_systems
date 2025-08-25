#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 256
#define MAX_ARGS 64

void parse_cmd(char *cmd, char **args) {
    int i = 0;
    char *token = strtok(cmd, " ");
    while (token && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int main() {
    char line[MAX_LINE];

    while (1) {
        printf("mysh> ");
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) break;
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, "exit") == 0) break;

        // ищем конвейер
        char *cmd1 = strtok(line, "|");
        char *cmd2 = strtok(NULL, "|");

        if (cmd2) {
            // если есть пайп
            int fd[2];
            pipe(fd);

            pid_t pid1 = fork();
            if (pid1 == 0) {
                // процесс 1 -> stdout в pipe
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]); close(fd[1]);

                char *args1[MAX_ARGS];
                parse_cmd(cmd1, args1);
                execvp(args1[0], args1);
                perror("exec1");
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                // процесс 2 <- stdin из pipe
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]); close(fd[1]);

                char *args2[MAX_ARGS];
                parse_cmd(cmd2, args2);
                execvp(args2[0], args2);
                perror("exec2");
                exit(1);
            }

            close(fd[0]); close(fd[1]);
            wait(NULL);
            wait(NULL);
        } else {
            // обычная команда без |
            char *args[MAX_ARGS];
            parse_cmd(line, args);

            if (args[0] == NULL) continue;

            pid_t pid = fork();
            if (pid == 0) {
                execvp(args[0], args);
                perror("exec");
                exit(1);
            }
            wait(NULL);
        }
    }
    return 0;
}

