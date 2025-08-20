#include <ncurses.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAX_FILES 1024

typedef struct {
    char *files[MAX_FILES];
    int is_dir[MAX_FILES];
    int count;
    int selected;
    char path[PATH_MAX];
} Panel;

void load_dir(Panel *p, const char *path) {
    DIR *d = opendir(path);
    if (!d) return;
    struct dirent *entry;
    p->count = 0;

    // сохранить путь
    realpath(path, p->path);


    p->files[p->count] = strdup("..");
    p->is_dir[p->count++] = 1;

    while ((entry = readdir(d)) != NULL && p->count < MAX_FILES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        p->files[p->count] = strdup(entry->d_name);

        char fullpath[PATH_MAX];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISDIR(st.st_mode))
            p->is_dir[p->count] = 1;
        else
            p->is_dir[p->count] = 0;

        p->count++;
    }
    closedir(d);
    p->selected = 0;
}

void free_panel(Panel *p) {
    for (int i = 0; i < p->count; i++) free(p->files[i]);
}

void draw_panel(Panel *p, int x, int active) {
    int h, w;
    getmaxyx(stdscr, h, w);

    for (int i = 0; i < p->count && i < h - 2; i++) {
        if (i == p->selected && active) attron(A_REVERSE);

        if (p->is_dir[i]) {
            attron(A_BOLD);
            mvprintw(i + 1, x + 1, "%s/", p->files[i]);
            attroff(A_BOLD);
        } else {
            mvprintw(i + 1, x + 1, "%s", p->files[i]);
        }

        if (i == p->selected && active) attroff(A_REVERSE);
    }
    mvprintw(h - 1, x + 1, "[%s]", p->path);
}

int main() {
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    Panel left, right;
    load_dir(&left, ".");
    load_dir(&right, ".");

    int active = 0; // 0 - левая панель, 1 - правая

    int ch;
    while (1) {
        clear();
        draw_panel(&left, 0, active == 0);
        draw_panel(&right, COLS / 2, active == 1);
        refresh();

        ch = getch();
        Panel *p = active ? &right : &left;

        if (ch == 'q') break;
        else if (ch == '\t') active = !active;
        else if (ch == KEY_UP && p->selected > 0) p->selected--;
        else if (ch == KEY_DOWN && p->selected < p->count - 1) p->selected++;
        else if (ch == '\n') {
            char newpath[PATH_MAX];
            if (strcmp(p->files[p->selected], "..") == 0) {
                // подняться на уровень выше
                char *slash = strrchr(p->path, '/');
                if (slash && slash != p->path) *slash = '\0';
                else strcpy(p->path, "/");
                free_panel(p);
                load_dir(p, p->path);
            } else if (p->is_dir[p->selected]) {
                // открыть папку
                snprintf(newpath, PATH_MAX, "%s/%s", p->path, p->files[p->selected]);
                free_panel(p);
                load_dir(p, newpath);
            } else {
                // открыть файл через less
                snprintf(newpath, PATH_MAX, "%s/%s", p->path, p->files[p->selected]);
                endwin(); // выйти из ncurses перед запуском less
                char cmd[PATH_MAX + 10];
                snprintf(cmd, sizeof(cmd), "less \"%s\"", newpath);
                system(cmd);
                // восстановить ncurses
                initscr();
                noecho();
                cbreak();
                keypad(stdscr, TRUE);
                curs_set(0);
            }
        }
    }

    free_panel(&left);
    free_panel(&right);
    endwin();
    return 0;
}

