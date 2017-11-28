#define COL_SIZE 10
#define ROW_SIZE 10

int foo(unsigned int a, unsigned int b, unsigned int n){
    unsigned int sum = a;
    unsigned i,j;

    for(i = 0 ; i < COL_SIZE; i++){
        for(j = 0; j < ROW_SIZE; j++){
            sum = (sum+a)*b;
        }
    }
    return sum;
}

int main(){
    unsigned int a = 5;
    unsigned int b = 3;
    unsigned int sum = foo(a,b, 2);
    return 0;
}
