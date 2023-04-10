#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int occurences(char *str, int n)
{
    int count = 0;
    while (*str != '\0')
    {
        if (*str == n)
            count++;
        str++;
    }

    return count;
}

char *add_whitespaces(char *str, int occur)
{
    char *with_whitespaces = malloc(strlen(str) + (2 * occur) + 1);
    char *init = str;

    int index = 0;
    while (*str != '\0')
    {
        if (*str == '>' || *str == '<')
        {
            with_whitespaces[index++] = ' ';
            with_whitespaces[index++] = *str;
            with_whitespaces[index++] = ' ';
        }
        else
            with_whitespaces[index++] = *str;

        str++;
    }
    with_whitespaces[index] = '\0';
    free(init);
    return with_whitespaces;
}

int if_exists(char *str, int n)
{
    while (*str != '\0')
    {
        if (*str == n)
            return 1;
        str++;
    }
    return 0;
}