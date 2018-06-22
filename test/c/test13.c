#include <stdio.h>

unsigned test13(unsigned j, unsigned p) {
    unsigned foo = j;
    for (unsigned i = 0; i < 5; ++i) {
        for (unsigned k = 0; k < 5; ++k) {
            if (p == 5)
                foo++;
            else
                foo--;
        }
    }
    return foo;
}

int main() {
    int result = test13(10, 5);
    printf("%d\n", result);
}
