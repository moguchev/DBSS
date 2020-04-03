#include <stdio.h>

int main() {
    puts("stdio:");

    char a;
    a = getchar();
    putchar(a);
    printf("%s", "\nHello world!\n");

    char str[80];
    scanf("%s", str);
    printf("%s%c", str, '\n');
    return 0;
}