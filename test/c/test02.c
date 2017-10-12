int foo(unsigned int a, unsigned int b){
    unsigned int sum;
    if(a / 2 == 4)
        sum = a + b;
    else
        sum = a - b;
    return sum;
}

int main(){
    unsigned int a = 5;
    unsigned int b = 3;
    unsigned int sum = foo(a,b);
    return 0;
}
