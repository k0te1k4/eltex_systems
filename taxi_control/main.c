#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <poll.h>

#define MSGSZ 256

typedef struct {
    pid_t pid;
    int fd;
    int alive;
} driver_t;

static driver_t *drivers = NULL;
static size_t drivers_n = 0;
static size_t drivers_cap = 0;

static void die(const char *m) {
    perror(m);
    exit(1);
}

static int set_nonblock(int fd) {
    int fl = fcntl(fd, F_GETFL, 0);
    if (fl < 0) return -1;
    if (fcntl(fd, F_SETFL, fl | O_NONBLOCK) < 0) return -1;
    return 0;
}

/* ---------- driver process ---------- */

static int timer_set_secs(int tfd, int seconds) {
    struct itimerspec it;
    memset(&it, 0, sizeof(it));
    if (seconds > 0) it.it_value.tv_sec = seconds;
    return timerfd_settime(tfd, 0, &it, NULL);
}

static int timer_remaining_secs(int tfd) {
    struct itimerspec it;
    if (timerfd_gettime(tfd, &it) != 0) return 0;
    if (it.it_value.tv_nsec != 0) return (int)it.it_value.tv_sec + 1;
    return (int)it.it_value.tv_sec;
}

static void driver_loop(int sock_fd) {
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (tfd < 0) _exit(1);

    int busy = 0;

    struct pollfd pfds[2];
    pfds[0].fd = sock_fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = tfd;
    pfds[1].events = POLLIN;

    for (;;) {
        int r = poll(pfds, 2, -1);
        if (r < 0) {
            if (errno == EINTR) continue;
            _exit(1);
        }

        if (pfds[1].revents & POLLIN) {
            uint64_t exp = 0;
            ssize_t rr = read(tfd, &exp, sizeof(exp));
            if (rr < 0 && errno != EAGAIN) _exit(1);
            busy = 0;
            (void)timer_set_secs(tfd, 0);
        }

        if (pfds[0].revents & (POLLHUP | POLLERR | POLLNVAL)) {
            _exit(0);
        }

        if (pfds[0].revents & POLLIN) {
            char msg[MSGSZ];
            ssize_t n = recv(sock_fd, msg, sizeof(msg) - 1, 0);
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
                _exit(1);
            }
            if (n == 0) _exit(0);
            msg[n] = 0;

            if (strncmp(msg, "QUIT", 4) == 0) {
                (void)send(sock_fd, "Bye", 3, 0);
                _exit(0);
            }

            if (strncmp(msg, "STATUS", 6) == 0) {
                if (busy) {
                    int rem = timer_remaining_secs(tfd);
                    char resp[64];
                    snprintf(resp, sizeof(resp), "Busy %d", rem);
                    (void)send(sock_fd, resp, strlen(resp), 0);
                } else {
                    (void)send(sock_fd, "Available", 9, 0);
                }
                continue;
            }

            if (strncmp(msg, "TASK", 4) == 0) {
                char *p = msg + 4;
                while (*p && isspace((unsigned char)*p)) p++;
                char *end = NULL;
                long sec = strtol(p, &end, 10);
                if (end == p || sec <= 0 || sec > 86400) {
                    (void)send(sock_fd, "Error BadTimer", 14, 0);
                    continue;
                }

                if (busy) {
                    int rem = timer_remaining_secs(tfd);
                    char resp[64];
                    snprintf(resp, sizeof(resp), "Busy %d", rem);
                    (void)send(sock_fd, resp, strlen(resp), 0);
                } else {
                    busy = 1;
                    if (timer_set_secs(tfd, (int)sec) != 0) _exit(1);
                    (void)send(sock_fd, "OK", 2, 0);
                }
                continue;
            }

            (void)send(sock_fd, "Error UnknownCommand", 20, 0);
        }
    }
}

/* ---------- CLI ---------- */

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

static driver_t* find_driver(pid_t pid) {
    for (size_t i = 0; i < drivers_n; i++) {
        if (drivers[i].pid == pid) return &drivers[i];
    }
    return NULL;
}

static void reap_children(void) {
    for (;;) {
        int st = 0;
        pid_t p = waitpid(-1, &st, WNOHANG);
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

static void cmd_create_driver(void) {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sv) < 0) die("socketpair");

    if (set_nonblock(sv[0]) != 0) die("nonblock");
    if (set_nonblock(sv[1]) != 0) die("nonblock");

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

static void send_cmd(driver_t *d, const char *cmd) {
    if (!d || !d->alive) {
        printf("Error NoSuchDriver\n");
        return;
    }
    ssize_t r = send(d->fd, cmd, strlen(cmd), 0);
    if (r < 0) {
        if (errno == EPIPE || errno == ECONNRESET) {
            printf("Error DriverDead\n");
            d->alive = 0;
            close(d->fd);
            d->fd = -1;
        } else {
            printf("Error SendFailed\n");
        }
    }
}

static void cmd_send_task(pid_t pid, int sec) {
    driver_t *d = find_driver(pid);
    if (!d || !d->alive) {
        printf("Error NoSuchDriver\n");
        return;
    }
    char msg[MSGSZ];
    snprintf(msg, sizeof(msg), "TASK %d", sec);
    send_cmd(d, msg);
}

static void cmd_get_status(pid_t pid) {
    driver_t *d = find_driver(pid);
    if (!d || !d->alive) {
        printf("Error NoSuchDriver\n");
        return;
    }
    send_cmd(d, "STATUS");
}

static void cmd_get_drivers(void) {
    for (size_t i = 0; i < drivers_n; i++) {
        if (!drivers[i].alive) continue;
        send_cmd(&drivers[i], "STATUS");
    }
}

static char* next_tok(char **ps) {
    char *s = *ps;
    while (*s && isspace((unsigned char)*s)) s++;
    if (!*s) { *ps = s; return NULL; }
    char *st = s;
    while (*s && !isspace((unsigned char)*s)) s++;
    if (*s) *s++ = 0;
    *ps = s;
    return st;
}

static void handle_driver_readable(driver_t *d) {
    for (;;) {
        char resp[MSGSZ];
        ssize_t n = recv(d->fd, resp, sizeof(resp) - 1, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            d->alive = 0;
            close(d->fd);
            d->fd = -1;
            return;
        }
        if (n == 0) {
            d->alive = 0;
            close(d->fd);
            d->fd = -1;
            return;
        }
        resp[n] = 0;
        printf("%d: %s\n", d->pid, resp);
        fflush(stdout);
    }
}

static void shutdown_all(void) {
    for (size_t i = 0; i < drivers_n; i++) {
        if (!drivers[i].alive) continue;
        (void)send(drivers[i].fd, "QUIT", 4, 0);
    }
    for (size_t i = 0; i < drivers_n; i++) {
        if (drivers[i].fd >= 0) close(drivers[i].fd);
        drivers[i].fd = -1;
        drivers[i].alive = 0;
    }
    for (;;) {
        int st = 0;
        pid_t p = waitpid(-1, &st, 0);
        if (p < 0) {
            if (errno == EINTR) continue;
            break;
        }
    }
}

int main(void) {
    signal(SIGPIPE, SIG_IGN);

    char line[MSGSZ];

    for (;;) {
        reap_children();

        size_t nfds = 1;
        for (size_t i = 0; i < drivers_n; i++) if (drivers[i].alive) nfds++;

        struct pollfd *pfds = (struct pollfd*)calloc(nfds, sizeof(*pfds));
        if (!pfds) die("calloc");

        pfds[0].fd = STDIN_FILENO;
        pfds[0].events = POLLIN;

        size_t idx = 1;
        for (size_t i = 0; i < drivers_n; i++) {
            if (!drivers[i].alive) continue;
            pfds[idx].fd = drivers[i].fd;
            pfds[idx].events = POLLIN | POLLHUP | POLLERR;
            idx++;
        }

        printf("> ");
        fflush(stdout);

        int r = poll(pfds, (nfds_t)nfds, -1);
        if (r < 0) {
            free(pfds);
            if (errno == EINTR) continue;
            die("poll");
        }

        if (pfds[0].revents & POLLIN) {
            if (!fgets(line, sizeof(line), stdin)) {
                free(pfds);
                break;
            }

            char *p = line;
            char *cmd = next_tok(&p);
            if (cmd) {
                if (strcmp(cmd, "create_driver") == 0) {
                    cmd_create_driver();
                } else if (strcmp(cmd, "send_task") == 0) {
                    char *spid = next_tok(&p);
                    char *st = next_tok(&p);
                    if (!spid || !st) {
                        printf("Error Usage: send_task <pid> <task_timer>\n");
                    } else {
                        pid_t pid = (pid_t)strtol(spid, NULL, 10);
                        int t = (int)strtol(st, NULL, 10);
                        if (pid <= 0 || t <= 0) printf("Error BadArgs\n");
                        else cmd_send_task(pid, t);
                    }
                } else if (strcmp(cmd, "get_status") == 0) {
                    char *spid = next_tok(&p);
                    if (!spid) {
                        printf("Error Usage: get_status <pid>\n");
                    } else {
                        pid_t pid = (pid_t)strtol(spid, NULL, 10);
                        if (pid <= 0) printf("Error BadArgs\n");
                        else cmd_get_status(pid);
                    }
                } else if (strcmp(cmd, "get_drivers") == 0) {
                    cmd_get_drivers();
                } else if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
                    free(pfds);
                    break;
                } else {
                    printf("Error UnknownCommand\n");
                }
                fflush(stdout);
            }
        }

        idx = 1;
        for (size_t i = 0; i < drivers_n; i++) {
            if (!drivers[i].alive) continue;

            if (pfds[idx].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                drivers[i].alive = 0;
                close(drivers[i].fd);
                drivers[i].fd = -1;
                idx++;
                continue;
            }
            if (pfds[idx].revents & POLLIN) {
                handle_driver_readable(&drivers[i]);
            }
            idx++;
        }

        free(pfds);
    }

    shutdown_all();
    free(drivers);
    return 0;
}
