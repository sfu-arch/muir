#include <stdio.h>
//#include <cilk/cilk.h>
#include "roi.h"

__attribute__((always_inline)) inline int gcd(int n1, int n2) {
    while (n1 != n2) {
        if (n1 > n2)
            n1 -= n2;
        else
            n2 -= n1;
    }
    return n1;
}

int gcd_cilk(int n[][2], int size) {
    //__app_roi_begin();
    for (int i = 0; i < size; ++i) {
        n[i][0] = gcd(n[i][0], n[i][1]);
    }
    //__app_roi_end();
    return 1;
}

int main() {
    int n[4][2] = {{10, 20}, {7, 21}, {81, 27}, {24, 36}};

    int result = gcd_cilk(n, 4);
    for (int i = 0; i < 4; i++) {
        printf("%d\n", n[i][0]);
    }
}
