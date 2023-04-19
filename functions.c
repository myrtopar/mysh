#include "functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define READ 0
#define WRITE 1

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

    pid_t pid;
    int status, exit_status;
    if ((pid = fork()) < 0)
        perror("fork failed");

    if (pid == 0) // child process
    {
        execvp(token_array[0], token_array);

        perror("Error executing command");
        printf("Error code: %d\n", errno);
        exit(EXIT_FAILURE);
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

void handle_redirections(char **token_array)
{
    int fd_in, fd_out;
    pid_t pid;
    int status, exit_status;

    for (int i = 0; token_array[i] != NULL; i++)
    {

        if (!strcmp(token_array[i], ">") || !strcmp(token_array[i], ">>")) // redirect output
        {

            if (!strcmp(token_array[i], ">"))
                fd_out = open(token_array[i + 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            else if (!strcmp(token_array[i], ">>"))
                fd_out = open(token_array[i + 1], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            token_array[i] = NULL;

            if ((pid = fork()) < 0)
                perror("fork failed");

            if (pid == 0) // child process
            {
                dup2(fd_out, STDOUT_FILENO); // redirect stdout to the file opened for output
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
                dup2(STDOUT_FILENO, fd_out); // restore the STDOUT_FILENO fd
                if (STDOUT_FILENO != fd_out)
                    close(fd_out);

                break;
            }
        }
        else if (!strcmp(token_array[i], "<"))
        {
            fd_in = open(token_array[i + 1], O_RDONLY);
            token_array[i] = NULL;

            if ((pid = fork()) < 0)
                perror("fork failed");

            if (pid == 0) // child process
            {

                dup2(fd_in, STDIN_FILENO); // redirect stdin to the file opened for input

                if (token_array[i + 2] != NULL)
                {
                    if (!strcmp(token_array[i + 2], ">"))
                    {
                        fd_out = open(token_array[i + 3], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        dup2(fd_out, STDOUT_FILENO); // redirect stdout to the file opened for output
                    }
                    else if (!strcmp(token_array[i + 2], ">>"))
                    {
                        fd_out = open(token_array[i + 3], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        dup2(fd_out, STDOUT_FILENO); // redirect stdout to the file opened for output
                    }
                }

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
                dup2(STDIN_FILENO, fd_in); // restore the STDIN_FILENO fd
                if (STDIN_FILENO != fd_in)
                    close(fd_in);
                break;
            }
        }
    }
    return;
}

void handle_pipes(char **token_array, int num_pipes, int token_count)
{
    int fds[num_pipes][2]; // 2d array, kathe grammi einai kai ena pipe, pou exei 2 ints, to write fd kai to read fd
    pid_t pid;
    int status;
    null_delim(token_array);

    // Creating all the pipes needed
    for (int i = 0; i < num_pipes; i++)
    {
        if (pipe(fds[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Forks. Kathe paidi tha klironomisei ola ta parapanw pipes
    for (int i = 0; i <= num_pipes; i++) //<= num_pipes giati ta processes einai kata 1 parapanw apo ta pipes
    {
        if ((pid = fork()) < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        { // child process. Tha kanw ta aparaitita dup2 kai meta exec

            if (i == 0) // stin prwti diergasia arkei na anakateuthynw mono to output. Den exw na parw input apo kapou
                dup2(fds[0][WRITE], STDOUT_FILENO);
            else if (i == num_pipes) // antistoixa gia tin teleutaia diergasia, anakateuthynw mono to input tou proigoumenou apo to pipe. To output de me noiazei paei kanonika stdout
                dup2(fds[i - 1][READ], STDIN_FILENO);
            else
            { // diaforetika enwnw kai ta 2 akra tou pipe me to process
                dup2(fds[i][WRITE], STDOUT_FILENO);
                dup2(fds[i - 1][READ], STDIN_FILENO);
            }

            // afou ekana ola ta dup2, kleinw ola ta pipe ends
            for (int j = 0; j < num_pipes; j++)
            {
                close(fds[j][READ]);
                close(fds[j][WRITE]);
            }

            int index = command_to_exec(token_array, i, token_count);
            execvp(token_array[index], &token_array[index]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // kleinw ola ta pipe ends sto parent process
    for (int i = 0; i < num_pipes; i++)
    {
        close(fds[i][READ]);
        close(fds[i][WRITE]);
    }

    for (int i = 0; i <= num_pipes; i++)
    {
        wait(&status);
    }
}

int command_to_exec(char **token_array, int nth_command, int token_count)
{
    // epistrefei to index tou stoixeioy pou ksekinaei to command gia to exec
    // no of command == how many nulls i must encounter
    int null_count = 0;
    for (int i = 0; i < token_count; i++)
    {
        if (null_count == nth_command)
            return i;
        if (token_array[i] == NULL)
            null_count++;
    }
    return 0;
}

void null_delim(char **array)
{
    // pairnei to token array kai vazei null opou exei | kai etsi diaxwrizei tis entoles metaksy tous gia to exec meta
    int i = 0;
    while (array[i] != NULL)
    {
        if (!strcmp(array[i], "|"))
        {
            array[i] = NULL;
        }
        i++;
    }
}
