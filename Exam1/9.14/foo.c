#include <stdio.h>

void foo(void) {
    int x = 0; static int y = 0;
    x += 1; y += 1;
    printf("x = %d y = %d\n",x,y);
}
int main(void) {
    foo();
    foo();
    foo();
    foo();
}