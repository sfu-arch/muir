int bar(int a, int b){
    return a + b;
}

int foo(int *c){
    int a = c[0];
    int b = c[1];
    int res = bar(a,b);
    return res;
}
