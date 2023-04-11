#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "functions.h"

#define MAX_CHAR_INPUT 300
#define MAX_ARGS 30

int main()
{

    char *input = malloc(MAX_CHAR_INPUT);
    char *token_array[MAX_ARGS];
    int count = 0;

    while (1)
    {
        printf("in-mysh-now:> ");
        fflush(stdout);

        fgets(input, MAX_CHAR_INPUT, stdin);

        char *ptrchr;
        if ((ptrchr = strchr(input, '\n')) != NULL)
            *ptrchr = '\0';

        if ((ptrchr = strchr(input, '\r')) != NULL)
            *ptrchr = '\0';

        int occ = occurences(input, '>') + occurences(input, '<') + occurences(input, '|');

        int redirect_output_flag = if_exists(input, '>');
        int redirect_input_flag = if_exists(input, '<');
        int pipe_flag = if_exists(input, '|');

        input = add_whitespaces(input, occ);

        // parse me strtok to input pou pira me tin fgets
        // to token_array exei ta tokens tou input spasmena: px gia entolh input ls -al: token_array[0] = ls, token_array[1] = -al
        int count = 0;
        char *token = strtok(input, " \t");
        while (token != NULL)
        {
            token_array[count] = token;
            count++;
            token = strtok(NULL, " \t");
        }

        if (count == 0) // αν η γραμμη δεν εχει κανενα token, προχώρα
            continue;

        // for (int i = 0; i < count; i++)
        // {
        //     fprintf(stdout, "%s\n", token_array[i]);
        // }

        token_array[count++] = NULL;

        if (!strcmp(token_array[0], "exit")) // exit shell
        {
            printf("logout\n");
            fflush(stdout);
            break;
        }

        if (!redirect_input_flag && !redirect_output_flag && !pipe_flag) // executing simple commands
        {
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
                continue;
            }
        }

        else if (redirect_input_flag || redirect_output_flag) // handling redirections
        {
            int fd, flag = 0; // flag -> not encountered a > yet. Means that the tokens are part of the command
            char *to_exec[count];
            pid_t pid;
            int status, exit_status;

            for (int i = 0; token_array[i] != NULL; i++)
            {
                // fprintf(stdout, "token %d: %s, flag: %d\n", i, token_array[i], flag);

                if (!strcmp(token_array[i], ">")) // redirect output
                {
                    // fprintf(stdout, "in >, about to open %s\n", token_array[i + 1]);
                    flag = 1;
                    fd = open(token_array[i + 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    to_exec[i] = NULL;

                    if ((pid = fork()) < 0)
                        perror("fork failed");

                    if (pid == 0) // child process
                    {
                        // for (int j = 0; j < count; j++)
                        // {
                        //     fprintf(stdout, " %d %s\n", j, to_exec[j]);
                        // }

                        dup2(fd, STDOUT_FILENO); // redirect stdout to the file opened for output
                        if (execvp(to_exec[0], to_exec) < 0)
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
                        dup2(STDOUT_FILENO, fd);
                        if (STDOUT_FILENO != fd)
                            close(fd);

                        // continue;
                    }
                }
                else if (flag == 0)
                {
                    to_exec[i] = token_array[i];
                }
            }
        }
    }

    return 0;
}
