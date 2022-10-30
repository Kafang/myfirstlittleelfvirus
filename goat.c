#include<stdio.h>
#include<stdlib.h> 
int main() {
    puts("Hello World\n");
    puts("Hello World\n");
    void* test = malloc(1337);
    free(test);
}
