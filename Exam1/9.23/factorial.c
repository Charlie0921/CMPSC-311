#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int factorial(int i) {
    assert(i >= 0);
    if (i <= 1) {
        return(i);
    }
    return factorial(i-1)*i;
}

int main(int argc, char** argv) {
    int i = 5;
    if (argc > 1) {
        i = atoi(argv[1]);
    }
    printf("Factorial of %d: %d\n",i,factorial(i));
    return 0;
}