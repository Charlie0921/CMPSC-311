char arr[4] = {'a', 'b', 'c', 'd'};
void *ptr = arr;
printf( 'As pointer : %p\n', ptr);
printf( 'As character : %c\n', *((char *)ptr));
printf( 'As 32 bit int : %d\n', *((int32_t *)ptr));