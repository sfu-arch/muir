int foo(unsigned int a, unsigned int b){
    int sum[2];
    sum[0] = a;
    sum[1] = b;
    return sum[0] + sum[1];
}

int main(){
    unsigned a = 5;
    unsigned b = 3;
    unsigned sum = foo(a,b);
    return 0;
}
