#include <iostream>

int main (int argc,  char** argv)
{
    int MAX = 100000;
    for (int i = 0; i < MAX; i ++) {
        for (int j = 0; j < MAX; j++) {
            for (int k = 0; k < MAX; k++) {
                int tmp = i * j * k;
                tmp += 1;
            }
        }
    }

    return 0;
}
