void* Mat(void* a, void* b);

void* test01(int a, int b){
    return  Mat(&a , &b);
}

int main(){
    int a = 5;
    int b = 3;

    void* sum = test01(a,b);

    return 0;
}
