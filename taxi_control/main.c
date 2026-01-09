#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <ctype.h>

typedef struct {
    pid_t pid;
    int fd;     // parent side
    int alive;
} driver_t;

static driver_t *drivers = NULL;
static size_t drivers_n = 0;
static size_t drivers_cap = 0;

static void die(const char *m) {
    perror(m);
    exit(1);
}

static int write_all(int fd, const void *buf, size_t n) {
    const char *p = (const char*)buf;
    while (n) {
        ssize_t r = write(fd, p, n);
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        p += (size_t)r;
        n -= (size_t)r;
    }
    return 0;
}

static int read_line(int fd, char *buf, size_t cap) {
    if (cap == 0) return -1;
    size_t i = 0;
    while (i + 1 < cap) {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r == 0) break;
        if (r < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        buf[i++] = c;
        if (c == '\n') break;
    }
    buf[i] = 0;
    return (int)i;
}

static double now_monotonic(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static int ceil_positive(double x) {
    if (x <= 0.0) return 0;
    int i = (int)x;
    if ((double)i == x) return i;
    return i + 1;
}

/* ---------------- driver process ---------------- */

static void driver_loop(int fd) {
    double busy_until = 0.0;

    for (;;) {
        char line[256];
        int n = read_line(fd, line, sizeof(line));
        if (n <= 0) _exit(0);

        // trim
        char *s = line;
        while (*s && isspace((unsigned char)*s)) s++;
        char *e = s + strlen(s);
        while (e > s && isspace((unsigned char)e[-1])) e--;
        *e = 0;

        double now = now_monotonic();
        int busy = (now < busy_until);
        int rem = ceil_positive(busy_until - now);

        if (strncmp(s, "TASK", 4) == 0 && (s[4] == 0 || isspace((unsigned char)s[4]))) {
            char *p = s + 4;
            while (*p && isspace((unsigned char)*p)) p++;
            long t = strtol(p, NULL, 10);
            if (t <= 0) {
                const char *resp = "Error BadTimer\n";
                write_all(fd, resp, strlen(resp));
                continue;
            }

            now = now_monotonic();
            busy = (now < busy_until);
            rem = ceil_positive(busy_until - now);

            if (busy) {
                char resp[64];
                snprintf(resp, sizeof(resp), "Busy %d\n", rem);
                write_all(fd, resp, strlen(resp));
            } else {
                busy_until = now + (double)t;
                const char *resp = "OK\n";
                write_all(fd, resp, strlen(resp));
            }
        } else if (strcmp(s, "STATUS") == 0) {
            now = now_monotonic();
            busy = (now < busy_until);
            rem = ceil_positive(busy_until - now);

            if (busy) {
                char resp[64];
                snprintf(resp, sizeof(resp), "Busy %d\n", rem);
                write_all(fd, resp, strlen(resp));
            } else {
                const char *resp = "Available\n";
                write_all(fd, resp, strlen(resp));
            }
        } else if (strcmp(s, "QUIT") == 0) {
            const char *resp = "Bye\n";
            write_all(fd, resp, strlen(resp));
            _exit(0);
        } else {
            const char *resp = "Error UnknownCommand\n";
            write_all(fd, resp, strlen(resp));
        }
    }
}

/* ---------------- parent / CLI ---------------- */

static void reap_children(void) {
    for (;;) {
        int status;
        pid_t p = waitpid(-1, &status, WNOHANG);
        if (p <= 0) break;
        for (size_t i = 0; i < drivers_n; i++) {
            if (drivers[i].alive && drivers[i].pid == p) {
                drivers[i].alive = 0;
                if (drivers[i].fd >= 0) close(drivers[i].fd);
                drivers[i].fd = -1;
                break;
            }
        }
    }
}

static driver_t* find_driver(pid_t pid) {
    for (size_t i = 0; i < drivers_n; i++) {
        if (drivers[i].pid == pid) return &drivers[i];
    }
    return NULL;
}

static void drivers_push(pid_t pid, int fd) {
    if (drivers_n == drivers_cap) {
        size_t nc = drivers_cap ? drivers_cap * 2 : 8;
        driver_t *nd = (driver_t*)realloc(drivers, nc * sizeof(driver_t));
        if (!nd) die("realloc");
        drivers = nd;
        drivers_cap = nc;
    }
    drivers[drivers_n++] = (driver_t){ .pid = pid, .fd = fd, .alive = 1 };
}

static int send_cmd_read_resp(driver_t *d, const char *cmd, char *resp, size_t resp_cap) {
    if (!d || !d->alive || d->fd < 0) return -1;

    if (write_all(d->fd, cmd, strlen(cmd)) != 0) return -1;

    int n = read_line(d->fd, resp, resp_cap);
    if (n <= 0) return -1;
    return 0;
}

static void cmd_create_driver(void) {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) die("socketpair");

    pid_t pid = fork();
    if (pid < 0) die("fork");

    if (pid == 0) {
        close(sv[0]);
        driver_loop(sv[1]);
        _exit(0);
    }

    close(sv[1]);
    drivers_push(pid, sv[0]);
    printf("%d\n", pid);
    fflush(stdout);
}

static void cmd_send_task(pid_t pid, int t) {
    driver_t *d = find_driver(pid);
    if (!d || !d->alive) {
        printf("Error NoSuchDriver\n");
        return;
    }
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "TASK %d\n", t);

    char resp[128];
    if (send_cmd_read_resp(d, cmd, resp, sizeof(resp)) != 0) {
        printf("Error DriverDead\n");
        d->alive = 0;
        if (d->fd >= 0) close(d->fd);
        d->fd = -1;
        return;
    }
    fputs(resp, stdout);
}

static void cmd_get_status(pid_t pid) {
    driver_t *d = find_driver(pid);
    if (!d || !d->alive) {
        printf("Error NoSuchDriver\n");
        return;
    }
    char resp[128];
    if (send_cmd_read_resp(d, "STATUS\n", resp, sizeof(resp)) != 0) {
        printf("Error DriverDead\n");
        d->alive = 0;
        if (d->fd >= 0) close(d->fd);
        d->fd = -1;
        return;
    }
    fputs(resp, stdout);
}

static void cmd_get_drivers(void) {
    for (size_t i = 0; i < drivers_n; i++) {
        driver_t *d = &drivers[i];
        if (!d->alive) continue;

        char resp[128];
        if (send_cmd_read_resp(d, "STATUS\n", resp, sizeof(resp)) != 0) {
            d->alive = 0;
            if (d->fd >= 0) close(d->fd);
            d->fd = -1;
            continue;
        }

        // resp already has \n
        printf("%d: %s", d->pid, resp);
    }
}

static void shutdown_all(void) {
    for (size_t i = 0; i < drivers_n; i++) {
        if (!drivers[i].alive || drivers[i].fd < 0) continue;
        char resp[64];
        send_cmd_read_resp(&drivers[i], "QUIT\n", resp, sizeof(resp));
        close(drivers[i].fd);
        drivers[i].fd = -1;
        drivers[i].alive = 0;
    }

    for (;;) {
        int status;
        pid_t p = waitpid(-1, &status, 0);
        if (p < 0) {
            if (errno == EINTR) continue;
            break;
        }
    }
}

static char* next_tok(char **ps) {
    char *s = *ps;
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) { *ps = s; return NULL; }
    char *start = s;
    while (*s && !isspace((unsigned char)*s)) s++;
    if (*s) *s++ = 0;
    *ps = s;
    return start;
}

int main(void) {
    signal(SIGPIPE, SIG_IGN);

    char line[256];

    for (;;) {
        reap_children();

        printf("> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) break;

        char *p = line;
        char *cmd = next_tok(&p);
        if (!cmd) continue;

        if (strcmp(cmd, "create_driver") == 0) {
            cmd_create_driver();
        } else if (strcmp(cmd, "send_task") == 0) {
            char *spid = next_tok(&p);
            char *st = next_tok(&p);
            if (!spid || !st) {
                printf("Error Usage: send_task <pid> <task_timer>\n");
                continue;
            }
            pid_t pid = (pid_t)strtol(spid, NULL, 10);
            int t = (int)strtol(st, NULL, 10);
            if (pid <= 0 || t <= 0) {
                printf("Error BadArgs\n");
                continue;
            }
            cmd_send_task(pid, t);
        } else if (strcmp(cmd, "get_status") == 0) {
            char *spid = next_tok(&p);
            if (!spid) {
                printf("Error Usage: get_status <pid>\n");
                continue;
            }
            pid_t pid = (pid_t)strtol(spid, NULL, 10);
            if (pid <= 0) {
                printf("Error BadArgs\n");
                continue;
            }
            cmd_get_status(pid);
        } else if (strcmp(cmd, "get_drivers") == 0) {
            cmd_get_drivers();
        } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            break;
        } else {
            printf("Error UnknownCommand\n");
        }
    }

    shutdown_all();
    free(drivers);
    return 0;
}
