#include <stdio.h>
#include <string.h> // memcpy

#define DATA_SIZE 20

float test25(float *a, int n, float mean) {
    float result = 0;
    for (int k = 0; k < n; ++k) {
        a[k] = a[k]/mean;
    }
    a[n - 1]++;
    return a[n-1];
}

int main() {
    float a[DATA_SIZE];
    float sum = 0.0;
    for(int i = 0; i < DATA_SIZE; i++){
        a[i] = (float)i;
        sum += (float)i;
    }
    for (int i = 0; i < DATA_SIZE; i++) {
        unsigned int ui;
        memcpy(&ui, &a[i], sizeof(ui));
        printf("%x\n", ui);
    }
    float foo = test25(a, DATA_SIZE, (float)(sum/DATA_SIZE));
    for (int i = 0; i < DATA_SIZE; i++) {
        unsigned int ui;
        memcpy(&ui, &a[i], sizeof(ui));
        printf("%x\n", ui);
    }
    printf("\nreturned: %x\n", *(unsigned int*)&foo);
    return (0);
}
