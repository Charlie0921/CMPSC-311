#include <stdio.h>

int x = 42;

int triple(void);

int main(void) {
    printf("%d\n",x);
    printf("%d\n",triple());
}

//gcc -Wall main.c triple.c -o foo  