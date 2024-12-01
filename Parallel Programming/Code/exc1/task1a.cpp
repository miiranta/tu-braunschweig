#include <iostream>

int sumByValue(int a, int b){
    return a+b;
}

void sumByReference(int &a, int &b, int &ret){
    ret = a + b;
}

void sumByPointer(int *a, int *b, int *ret){
    *ret = *a + *b;
}

int main() {
    
    int a = 1, b = 1, aa = 1, bb = 1, aaa = 1, bbb = 1;
    int res1, res2, res3;

    res1 = sumByValue(a, b);
    printf("%d\n", res1);

    sumByReference(aa, bb, res2);
    printf("%d\n", res2);

    sumByPointer(&aaa, &bbb, &res3);
    printf("%d\n", res3);

}

