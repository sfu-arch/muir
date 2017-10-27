int foo(unsigned int a, unsigned int b, int n){
    unsigned int sum = a;
    int i;

    for(i = 0 ; i < n; i++)
        sum = (sum+a)*b;
    return sum;
}

int main(){
    unsigned int a = 5;
    unsigned int b = 3;
    unsigned int sum = foo(a,b, 2);
    return 0;
}
