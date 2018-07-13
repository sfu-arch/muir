#include <stdio.h>

unsigned test14(unsigned *a, unsigned i) {
    if (i % 2)
        a[0] = 4;

    a[0] = 5;
    return 1;
}

int main() {
    unsigned a[1] = {0};
    test14(a, 5);
    printf("%d\n", a[0]);
}
