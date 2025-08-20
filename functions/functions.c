#include <stdio.h>
#include <string.h>

#define MAX_ABONENTS 100
#define MAX_LEN 10

struct abonent {
    char name[MAX_LEN];
    char second_name[MAX_LEN];
    char tel[MAX_LEN];
};

struct abonent abonent_book[MAX_ABONENTS];
int abonent_count = 0;


void add_abonent() {
    if (abonent_count >= MAX_ABONENTS) {
        printf("Место в книге закончилось. Для добавления новых абонентов удалите старых\n");
        return;
    }
    printf("Введите имя абонента (максимум %d символов):\n", MAX_LEN - 1);
    scanf("%s", abonent_book[abonent_count].name);
    printf("Введите фамилию абонента (максимум %d символов):\n", MAX_LEN - 1);
    scanf("%s", abonent_book[abonent_count].second_name);
    printf("Введите номер телефона абонента (максимум %d символов):\n", MAX_LEN - 1);
    scanf("%s", abonent_book[abonent_count].tel);
    abonent_count++;
}

void remove_abonent() {
    int number = 0;
    printf("Введите номер удаляемого абонента (-1 для отмены):\n");
    scanf("%d", &number);
    if (number < 0 || number >= abonent_count) {
        printf("Неверно указан номер или отмена удаления\n");
        return;
    }

    for (int i = number; i < abonent_count - 1; i++) {
        abonent_book[i] = abonent_book[i + 1];
    }
    abonent_count--;
    printf("Абонент удалён\n");
}

void search_abonent() {
    char name[MAX_LEN];
    printf("Введите имя абонента (максимум %d символов):\n", MAX_LEN - 1);
    scanf("%s", name);

    int counter = 0;
    for (int i = 0; i < abonent_count; i++) {
        if (strcmp(abonent_book[i].name, name) == 0) {
            printf("%d: %s %s %s\n", i, abonent_book[i].name, abonent_book[i].second_name, abonent_book[i].tel);
            counter++;
        }
    }
    if (counter)
        printf("Всего найдено %d абонент(ов)\n", counter);
    else
        printf("Абонент с таким именем не найден\n");
}

void print_book() {
    if (abonent_count == 0) {
        printf("В книге нет абонентов\n");
        return;
    }
    printf("Всего в книге %d абонент(ов):\n", abonent_count);
    for (int i = 0; i < abonent_count; i++) {
        printf("%d: %s %s %s\n", i, abonent_book[i].name, abonent_book[i].second_name, abonent_book[i].tel);
    }
}


void print_menu() {
    printf("\nВыберите действие:\n"
           "1) Добавить абонента\n"
           "2) Удалить абонента\n"
           "3) Поиск абонентов по имени\n"
           "4) Вывод всех записей\n"
           "5) Выход\n");
}

void handle_choice(int choice) {
    switch (choice) {
        case 1: add_abonent(); break;
        case 2: remove_abonent(); break;
        case 3: search_abonent(); break;
        case 4: print_book(); break;
        case 5: printf("Выход\n"); break;
        default: printf("Неверный выбор\n"); break;
    }
}


int main() {
    int choice = 0;
    do {
        print_menu();
        scanf("%d", &choice);
        handle_choice(choice);
    } while (choice != 5);
    return 0;
}

