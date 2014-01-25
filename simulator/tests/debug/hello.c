#include <stdio.h>

int main(int argc, char** argv) {
    printf("Hello World: %d\n", argc);
    int i = 0;
    int j = 0;

    i = i + 1;
    printf("i=%d, j=%d\n", i, j);
    
    j = i + 1;
    printf("i=%d, j=%d\n", i, j);

    i = i + 1;
    printf("i=%d, j=%d\n", i, j);
    
    j = i + 1;
    printf("i=%d, j=%d\n", i, j);

    i = i + 1;
    printf("i=%d, j=%d\n", i, j);
    
    j = i + 1;
    printf("i=%d, j=%d\n", i, j);

    return 0;
}
