#include <stdlib.h>
#include <stdio.h>

#define count 10


int main() {
    int a[count];
    int *pointer = (int *) &a;

    for (int i = 0; i < count; ++i) {
        a[i] = rand();
    }

    for (int i = 0; i < count; ++i) {
        printf("%d: %d\n",i,*pointer);
        pointer++;
    }
}