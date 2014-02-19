#include <iostream>

int main (int argc,  char** argv)
{
    int MAX = 100000;
    for (int i = 0; i < MAX; i ++) {
        for (int j = 0; j < MAX; j++) {
            for (int k = 0; k < MAX; k++) {
                i * j * k;
            }
        }
    }

    return 0;
}
