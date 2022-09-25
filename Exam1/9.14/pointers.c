int main(void) {
    int x = 42;
    int *p;

    p = &x;
     
    printf("x is %d\n",x);
    printf("&x is %p\n", &x);
    printf("p is %p\n", p);
    return 0;
}