#include <stdio.h>
#include <unistd.h>

int main()
{
    char input[100];
    printf("Enter some input: ");
    fgets(input, 100, stdin);
    // sleep(10);
    printf("You entered: %safter 10 secs\n", input);
    return 0;
}
