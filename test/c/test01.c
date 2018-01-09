void* Mat(void* a, void* b);

void* foo(int a, int b){
    return  Mat(&a , &b);
}

int main(){
    int a = 5;
    int b = 3;

    void* sum = foo(a,b);

    return 0;
}
