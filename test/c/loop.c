int foo(int *in, int *out, int gain, int nsamps) {
    int i;
    /*int sum[5];*/
    for (i = 0; i < nsamps; i++) {
        out[0] = in[i] * gain;
    }

    return out[0];

    /*out1[0] = sum[0] * 5;*/
    /*out2[0] = sum[0] * 8;*/
}

int main() {
    int *in, *out1, *out2;
    int gain;
    int nsamps;
    int a = foo(in, out1, gain, nsamps);

    return 0;
}
