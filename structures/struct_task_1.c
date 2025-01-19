#include <printf.h>
#include <string.h>


struct abonent {
    char name[10];
    char second_name[10];
    char tel[10]; };

int abonent_count = 0;
struct abonent abonent_book[100] = {0};


void add_abonent(){
    if(abonent_count >= 100){
        printf("Место в книге закончилось. Для добавления новых абонентов удалите старых\n");
        return;
    }
    printf("Введите имя абонента(максимум 9 символов):\n");
    scanf("%s",abonent_book[abonent_count].name);
    printf("Введите фамилию абонента(максимум 9 символов):\n");
    scanf("%s",abonent_book[abonent_count].second_name);
    printf("Введите номер телефона абонента(максимум 9 символов):\n");
    scanf("%s",abonent_book[abonent_count].tel);
    abonent_count++;
}

void remove_abonent(){
    int number = 0;
    printf("Введите номер удаляемого абонента(не совпадает с номером телефона, можно посмотреть в списке всех абонентов перед двоеточием)\n -1 для отказа от удаления\n");
    scanf("%d",&number);
    char empty_string[10] = {0};
    if(number >= 0 && number < abonent_count){
        memset(abonent_book[number].name,0,sizeof(abonent_book[number].name));
        memset(abonent_book[number].second_name,0,sizeof(abonent_book[number].second_name));
        memset(abonent_book[number].tel,0,sizeof(abonent_book[number].tel));
        for (int i = number+1; i < 100; ++i) {
            if(strcmp(abonent_book[i].name,empty_string) == 0){
                continue;
            } else{
                strcpy(abonent_book[i-1].name,abonent_book[i].name);
                strcpy(abonent_book[i-1].second_name,abonent_book[i].second_name);
                strcpy(abonent_book[i-1].tel,abonent_book[i].tel);
            }
        }
        abonent_count--;
    }
    else{
        printf("Неверно указан номер удаляемого абонента или вы отказались от удаления");
    }
}

void search_abonent(){
    char name[10] = {0};
    printf("Введите имя абонента(максимум 10 символов):\n");
    scanf("%s",name);
    int counter = 0;
    for (int i = 0; i < abonent_count; ++i) {
        if(strcmp(abonent_book[i].name,name) == 0){
            printf("%d: %s %s %s\n",i,abonent_book[i].name,abonent_book[i].second_name,abonent_book[i].tel);
            counter++;
        }
    }
    if(counter != 0) printf("Всего найдено %d абонентов\n",counter);
    else printf("Абонент с таким именем не найден\n");

}

void print_book(){
    if(abonent_count != 0) {
        printf("Всего в книге %d абонентов:\n", abonent_count);
        for (int i = 0; i < abonent_count; ++i) {
            printf("%d: %s %s %s\n", i, abonent_book[i].name, abonent_book[i].second_name, abonent_book[i].tel);
        }
    }
    else {
        printf("В книге нет абонентов\n");
    }
}


int main(){

    int choice = 0;
    char exit_flag = 1;

    while(exit_flag) {
        printf("1) Добавить абонента\n"
               "2) Удалить абонента\n"
               "3) Поиск абонентов по имени\n"
               "4) Вывод всех записей\n"
               "5) Выход\n");
        scanf("%d",&choice);
        switch (choice) {
            case 1:
                add_abonent();
                break;
            case 2:
                remove_abonent();
                break;
            case 3:
                search_abonent();
                break;
            case 4:
                print_book();
                break;
            case 5:
                printf("Выход\n");
                exit_flag = 0;
                break;
            default:
                continue;
        }
    }
}