#include <stdio.h>

int myfunc(int i) {
    printf("Got into function with %d\n",i);
    return 0;

}

int main(void) {
    myfunc(10);
    return 0;
}