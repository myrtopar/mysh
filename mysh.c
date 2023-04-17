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
#define READ 0
#define WRITE 1

int main()
{

    char command[MAX_CHAR_INPUT]; //= malloc(MAX_CHAR_INPUT);
    char *token_array[MAX_ARGS];
    int count = 0;

    while (1)
    {
        printf("in-mysh-now:> ");
        fflush(stdout);

        fgets(command, MAX_CHAR_INPUT, stdin);

        char *ptrchr;
        if ((ptrchr = strchr(command, '\n')) != NULL)
            *ptrchr = '\0';

        if ((ptrchr = strchr(command, '\r')) != NULL)
            *ptrchr = '\0';

        int occ = occurences(command, '>') + occurences(command, '<') + occurences(command, '|');

        int redirect_output_flag = if_exists(command, '>');
        int redirect_input_flag = if_exists(command, '<');
        int pipe_flag = if_exists(command, '|');

        char *input = add_whitespaces(command, occ);

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

        token_array[count++] = NULL;

        // for (int i = 0; i < count; i++)
        // {
        //     fprintf(stdout, "%s\n", token_array[i]);
        // }

        if (!strcmp(token_array[0], "exit")) // exit shell
        {
            printf("logout\n");
            fflush(stdout);
            break;
        }

        if (!redirect_input_flag && !redirect_output_flag && !pipe_flag) // executing simple commands
        {
            execute_simple_command(token_array);
        }

        else if (redirect_input_flag || redirect_output_flag !pipe_flag) // handling redirections ONLY
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

                        fprintf(stdout, "about to exec\n");
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
        }
        free(input);
    }

    return 0;
}