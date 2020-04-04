#include <stdio.h>

int main(int argc, char** argv){
    
    fprintf(stdout,"Программа %s запущена c аргуменами:",  argv[0]);
    for(size_t i = 0; argv[++i] != NULL;)
        printf(" %s", argv[i]);
    fprintf(stdout, "\n\r");

    char str[16];
    scanf("%s", str);
    fprintf(stdout, "STDOUT: %s%c", str, '\n');
    fprintf(stderr, "STDERR: %s%c", str, '\n');

    return 0;
}