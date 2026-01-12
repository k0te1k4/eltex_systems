#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Hello World\n");
    fflush(stdout);

    while (1) sleep(1);
    return 0;
}
