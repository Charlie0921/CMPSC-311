int main(void) {
    printf("sumTo (5) is : %d\n",sumTo(5));
    return 0;
}

int sumTo(int max) {
    int i , sum = 0;

    for (i = 1; i <= max; i++) {
        sum += i;
    }
    return sum;
}