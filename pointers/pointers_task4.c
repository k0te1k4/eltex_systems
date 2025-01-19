#include <stdio.h>


#define string_size 100

const char* check_string(const char* input_string,const char* substring){
    for (int i = 0; i < string_size; ++i) {
        if(input_string[i] == substring[0]) {
            for (int j = 0; j < string_size - i; ++j) {
                if(input_string[i+j]==substring[j]){
                    printf("%c",substring[j]);
                    if(substring[j+1] == '\n'){
                        printf("\nTask Complete\n");
                        return substring;
                    }
                } else break;
            }
        }
    }
    printf("\nTask failed\n");
    return NULL;
}


int main() {
    char input_string[string_size] = {0};
    char substring[string_size] = {0};

    printf("Введите строку:");
    fgets(input_string, sizeof(input_string), stdin);
    printf("Введите подстроку для поиска:");
    fgets(substring, sizeof(substring), stdin);

    printf("%s", check_string(input_string,substring));
}