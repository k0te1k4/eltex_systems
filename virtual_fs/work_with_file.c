#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    const char *filename = "output.txt";
    const char *text = "String from file";

    // Запись строки в файл
    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Ошибка открытия файла для записи");
        return 1;
    }
    fprintf(f, "%s", text);
    fclose(f);

    // Чтение строки из файла с конца
    f = fopen(filename, "r");
    if (!f) {
        perror("Ошибка открытия файла для чтения");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';

    printf("Прямой порядок: %s\n", buffer);
    printf("Обратный порядок: ");
    for (long i = size - 1; i >= 0; i--) {
        putchar(buffer[i]);
    }
    putchar('\n');

    free(buffer);
    fclose(f);
    return 0;
}

