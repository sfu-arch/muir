void foo(int *in, int *out1, int *out2, int gain, int nsamps) {
    int i;
    int sum[5];
    for (i = 0; i < nsamps; i++) {
        sum[0] = sum[0] + in[i] * gain;
    }

    out1[0] = sum[0] * 5;
    out2[0] = sum[0] * 8;
}

int main() {
    int *in, *out1, *out2;
    int gain;
    int nsamps;
    foo(in, out1, out2, gain, nsamps);

    return 0;
}
