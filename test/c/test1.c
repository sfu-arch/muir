void do_work(int i);

int foo(int *in, int *out, int gain) {
    int i;
    int sum;
    sum = sum + gain;
    for (int i = 0; i < 5; ++i) {
        do_work(i);
    }

    sum = sum + gain;
}

int main(){
    int *in;
    int *out;
    int gain;
    foo(in,out,gain);

    return 0;
}
