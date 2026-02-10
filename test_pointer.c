#include <stdio.h>

int main() {
    int *p;
    int a = 10;
    p = &a;
    printf("%d\n", *p);

    int **pp = &p;
    printf("%d\n", **pp);

    return 0;
}