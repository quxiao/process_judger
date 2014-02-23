#include <iostream>

int main (int argc,  char** argv)
{
    const int MAX_NUM = 10000;
    int arr[MAX_NUM][MAX_NUM][MAX_NUM];
    int a, b;
    while(std::cin >> a >> b) {
        std::cout << a+b << std::endl;
    }
    return 0;
}
