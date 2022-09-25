int sumAll(int a[5]);

int main(void) {
    int numbers[5] = {3,4,1,7,4};
    int sum = sumAll(numbers);
    return 0;
}

int sumAll(int a[5]) {
    int i , sum = 0;
    for (i = 0; i < 5; i++) {
        sum += a[i];
    }
    return sum;
}