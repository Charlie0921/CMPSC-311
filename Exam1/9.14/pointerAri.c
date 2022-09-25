#include <stdio.h>
int main(void) {
int foo[10] = { [0 ... 9] = 42 };
int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
for (int i = 0; i < 10; ++i)
sum1 += foo[i];

for (int *p = &foo[0]; p < &foo[10]; ++p)
sum2 += *p;

for (int *p = foo; p < foo + 10; ++p)
sum3 += *p;

int *p = foo;
for (int i = 0; i < 10; ++i)
sum4 += p[i];

printf("%d %d %d %d\n", sum1, sum2, sum3, sum4);
}