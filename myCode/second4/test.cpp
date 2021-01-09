#include <iostream>
using namespace std;
int main(){
    int a[2] = {0};
    int t = 0;
    a[t++] = 10;// 县运算玩 t+=1
    std::cout << a[0] << a[1] ;
}