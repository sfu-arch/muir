void foo(int *in, int *out1, int *out2, int gain, int nsamps) {
    int i;
    int sum = 0;
    for (i = 0; i < nsamps; i++) {
        sum = sum + in[i] * gain;
    }

    out1[0] = sum * 5;
    out2[0] = sum * 8;
}

int main() {
    int *in, *out1, *out2;
    int gain;
    int nsamps;
    foo(in, out1, out2, gain, nsamps);

    return 0;
}
