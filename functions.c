#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

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
        if (*str == '|' || *str == '<')
        {
            with_whitespaces[index++] = ' ';
            with_whitespaces[index++] = *str;
            with_whitespaces[index++] = ' ';
        }
        else if (*str == '>' && *(str + 1) == '>')
        {
            with_whitespaces[index++] = ' ';
            with_whitespaces[index++] = *str;
        }
        else if (*str == '>' && *(str - 1) == '>' && *(str + 1) != '>')
        {
            with_whitespaces[index++] = *str;
            with_whitespaces[index++] = ' ';
        }
        else
            with_whitespaces[index++] = *str;

        str++;
    }
    with_whitespaces[index] = '\0';
    // free(init);
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

void execute_simple_command(char **token_array)
{
    printf("in simple command\n");

    pid_t pid;
    int status, exit_status;
    if ((pid = fork()) < 0)
        perror("fork failed");

    if (pid == 0) // child process
    {
        if (execvp(token_array[0], token_array) < 0)
        {
            perror("Error executing command");
            printf("Error code: %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0) // parent process
    {
        if (waitpid(pid, &status, 0) < 0)
        {
            perror("waitpid() failed");
            exit(1);
        }
    }
}