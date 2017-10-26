<<<<<<< HEAD
void *Mat(void *, void *);

void* foo(int a, int b){
    return Mat(&a, &b);
=======

void* Mat(void* a, void* b);

void* foo(int a, int b){
    /* =int sum; */
    return  Mat(&a , &b);
>>>>>>> 5bc10d0db1059de80900a408d681d0e1c0d2a1d1
}

int main(){
    int a = 5;
    int b = 3;
<<<<<<< HEAD
    void *sum = foo(a,b);
=======
    void* sum = foo(a,b);
>>>>>>> 5bc10d0db1059de80900a408d681d0e1c0d2a1d1
    return 0;
}
