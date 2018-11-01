#include <stdio.h>

unsigned test21(unsigned *a, unsigned *b, unsigned n) {
    int result = 0;
    for (unsigned j = 0; j < n; ++j) {
        for (unsigned k = 0; k < n; ++k) {
            unsigned index = k + (j * n);
            a[index] = 2 * a[index];
        }
        b[j] = a[j*n];
    }
    return result;
}

int main() {
    unsigned a[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    unsigned b[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    int foo = test21(a, b, 3);
    for (int i = 0; i < 9; i++) {
        printf("%u,", a[i]);
    }
    printf("\nreturned: %u\n", foo);
    return (0);
}
