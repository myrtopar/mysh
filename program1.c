#include <stdio.h>

int main()
{
    char input[100];
    printf("Enter some input: ");
    fgets(input, 100, stdin);
    printf("You entered: %s", input);
    return 0;
}
