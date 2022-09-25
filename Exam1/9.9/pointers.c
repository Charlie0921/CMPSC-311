int main(void) {
    int i = 5;
    int *ip = &i;

    printf("%d\n",i);
    printf("%p\n",&i);
    printf("%p\n", &ip);
    *ip = 42;
    printf("%d\n", i);
    printf("%d\n",*ip);
}