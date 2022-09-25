#include <stdio.h>

int main(void) {
    int foo[10];
    int *p = foo; // point to the 0-th element
    printf("%p\n", p);
    ++p; // point to the next element
    printf("%p\n", p);
    char bar[10];
    char *q = bar; // point to the 0-th element
    printf("%p\n", q);
    ++q; // point to the next element
    printf("%p\n", q);
}