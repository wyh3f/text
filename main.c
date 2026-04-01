#include <stdio.h>

int add_12(int abb) {
    return abb*abb;
}


int main(void) {
    printf("Hello, World!\n");
    printf("how are you?\n");
    int i=0;
    for (i=1;i<100;i++) {
        printf("out %d =%d\n",i,add_12(i));
    }
    return 0;
}
