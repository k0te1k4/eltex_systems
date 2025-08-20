#include <stdio.h>
#include "add.h"
#include "sub.h"
#include "mul.h"
#include "div.h"

int main() {
    int choice;
    int a, b, result;

    while (1) {
        printf("\nМеню:\n");
        printf("1) Сложение\n");
        printf("2) Вычитание\n");
        printf("3) Умножение\n");
        printf("4) Деление\n");
        printf("5) Выход\n");
        printf("Выберите пункт: ");
        scanf("%d", &choice);

        if (choice == 5) {
            printf("Выход из программы.\n");
            break;
        }

        printf("Введите два целых числа: ");
        scanf("%d %d", &a, &b);

        switch (choice) {
            case 1:
                result = add(a, b);
                printf("Результат: %d\n", result);
                break;
            case 2:
                result = sub(a, b);
                printf("Результат: %d\n", result);
                break;
            case 3:
                result = mul(a, b);
                printf("Результат: %d\n", result);
                break;
            case 4:
                if (b == 0) {
                    printf("Ошибка: деление на ноль!\n");
                } else {
                    result = divide(a, b);
                    printf("Результат: %d\n", result);
                }
                break;
            default:
                printf("Неверный выбор!\n");
        }
    }

    return 0;
}

