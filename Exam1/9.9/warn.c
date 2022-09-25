#include <stdio.h>

int main(void) {
    int x,y = 5;
    long z = x+y;
    
    printf("z is '%ld' \n",z);
    {
        int y = 10;
        printf("y is '%d'\n",y);
    }
    int w = 20;
    printf("y is '%d', w is '%d'\n",y,w);
    return 0;
}