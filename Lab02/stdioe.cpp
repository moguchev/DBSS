#include <stdio.h>

int main() {
    char str[16];
    scanf("%s", str);
    fprintf(stdout, "STDOUT: %s%c", str, '\n');
    fprintf(stderr, "STDERR: %s%c", str, '\n');

    return 0;
}