#include <stdio.h>
#include <stdbool.h>

unsigned test19(unsigned *a, unsigned i, bool flag) {
    unsigned value = 0;
    if(flag)
        value = a[i];
    else
        a[i] = i;

    return value;
}

int main() {
    unsigned a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int foo = test19(a, 5, true);
    for (int i = 0; i < 8; i++) {
        printf("%u,", a[i]);
    }
    printf("\nreturned: %u\n", foo);
    return (0);
}
