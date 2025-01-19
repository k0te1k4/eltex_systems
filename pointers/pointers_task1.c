#include <stdio.h>

int main(void) {
    int a;
    scanf("%d", &a);

    unsigned char *ap = (unsigned char*) &a;
    unsigned char input = 0;
    scanf("%hhu", &input);

    ap[2] = input;
    printf("%d", a);
    return 0;
}
