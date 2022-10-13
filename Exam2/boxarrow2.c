int main(int argc, char **argv {
    int x = 1;
    int arr[3] = {2,3,4};
    int *p = &arr[1];
    int **dp = &p;

    **dp += 1;
    p += 1; 
    **dp += 1; 
    return 0;
}