int main(void) {
    int i = 5;
    int *ip = &i;

    printf("value of i: %d\n",i);
    printf("address of i: %p\n",&i);
    printf("value of i: %d\n", *ip);
    printf("address of ip: %p\n", &ip);
    *ip = 42;
    printf("%d\n", i);
    printf("%d\n",*ip);
}