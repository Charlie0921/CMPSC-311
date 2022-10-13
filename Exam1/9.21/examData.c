#include <stdio.h>
#include <stdint.h>

void show_bytes(uint8_t * start, int len) {
    for (int i = 0; i < len; ++i) {
        printf("p\t0x%.2x\n", start+i, start[i]);
    }
    printf("\n");
}

int main(void) {
    int a = 305419896;
    printf("a lives at address %p\n\n", &a);
    show_bytes((uint8_t *)&a, sizeof(int));
}