#include <stdio.h>

unsigned test18(unsigned *a, unsigned n) {
    int result = 0;
    for (unsigned k = 0; k < n; ++k) {
        a[k] = 2 * a[k];
    }
    a[n - 1]++;
    return a[n-1];
}

int main() {
    unsigned a[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    int foo = test18(a, 8);
    for (int i = 0; i < 8; i++) {
        printf("%u,", a[i]);
    }
    printf("\nreturned: %u\n", foo);
    return (0);
}
