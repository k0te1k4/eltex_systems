#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 10

struct abonent {
    char name[MAX_LEN];
    char second_name[MAX_LEN];
    char tel[MAX_LEN];
    struct abonent* next;
    struct abonent* prev;
};

struct abonent* head = NULL; // начало списка
struct abonent* tail = NULL; // конец списка


void add_abonent() {
    struct abonent* new_node = malloc(sizeof(struct abonent));
    if (!new_node) {
        printf("Ошибка выделения памяти!\n");
        return;
    }

    printf("Введите имя (до %d символов): ", MAX_LEN - 1);
    scanf("%s", new_node->name);
    printf("Введите фамилию (до %d символов): ", MAX_LEN - 1);
    scanf("%s", new_node->second_name);
    printf("Введите телефон (до %d символов): ", MAX_LEN - 1);
    scanf("%s", new_node->tel);

    new_node->next = NULL;
    new_node->prev = tail;

    if (tail) {
        tail->next = new_node;
    } else {
        head = new_node; // если список пуст
    }
    tail = new_node;

    printf("Абонент добавлен!\n");
}

void print_book() {
    if (!head) {
        printf("Справочник пуст.\n");
        return;
    }
    printf("Список абонентов:\n");
    struct abonent* current = head;
    int index = 0;
    while (current) {
        printf("%d: %s %s %s\n", index, current->name, current->second_name, current->tel);
        current = current->next;
        index++;
    }
}

void search_abonent() {
    if (!head) {
        printf("Справочник пуст.\n");
        return;
    }
    char name[MAX_LEN];
    printf("Введите имя для поиска: ");
    scanf("%s", name);

    struct abonent* current = head;
    int found = 0, index = 0;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            printf("%d: %s %s %s\n", index, current->name, current->second_name, current->tel);
            found++;
        }
        current = current->next;
        index++;
    }

    if (!found) {
        printf("Абонентов с именем '%s' не найдено.\n", name);
    } else {
        printf("Найдено %d совпадений.\n", found);
    }
}

void remove_abonent() {
    if (!head) {
        printf("Справочник пуст.\n");
        return;
    }
    int number;
    printf("Введите номер абонента для удаления: ");
    scanf("%d", &number);

    struct abonent* current = head;
    int index = 0;
    while (current && index < number) {
        current = current->next;
        index++;
    }

    if (!current) {
        printf("Абонент с таким номером не найден.\n");
        return;
    }

    if (current->prev) {
        current->prev->next = current->next;
    } else {
        head = current->next; // удаляемый элемент был первым
    }

    if (current->next) {
        current->next->prev = current->prev;
    } else {
        tail = current->prev; // удаляемый элемент был последним
    }

    free(current);
    printf("Абонент удалён.\n");
}

void free_all() {
    struct abonent* current = head;
    while (current) {
        struct abonent* next = current->next;
        free(current);
        current = next;
    }
    head = tail = NULL;
}


void print_menu() {
    printf("\nМеню:\n");
    printf("1) Добавить абонента\n");
    printf("2) Удалить абонента\n");
    printf("3) Поиск по имени\n");
    printf("4) Вывод всех абонентов\n");
    printf("5) Выход\n");
}

int main() {
    int choice;
    do {
        print_menu();
        scanf("%d", &choice);
        switch (choice) {
            case 1: add_abonent(); break;
            case 2: remove_abonent(); break;
            case 3: search_abonent(); break;
            case 4: print_book(); break;
            case 5: 
                free_all();
                printf("Завершение программы.\n");
                break;
            default: 
                printf("Неверный выбор!\n");
        }
    } while (choice != 5);
    return 0;
}

