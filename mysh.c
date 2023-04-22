#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include "functions.h"

#define MAX_CHAR_INPUT 300
#define MAX_ARGS 30

int main()
{
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);

    pid_t shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0)
    {
        perror("Couldn't put the shell in its own process group");
        exit(1);
    }
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    signal(SIGINT, SIG_DFL);
    // signal(SIGTSTP, SIG_DFL);

    char command[MAX_CHAR_INPUT]; //= malloc(MAX_CHAR_INPUT);
    char history[20][MAX_CHAR_INPUT];
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
        int num_pipes = occurences(command, '|');

        int redirect_output_flag = if_exists(command, '>');
        int redirect_input_flag = if_exists(command, '<');
        int pipe_flag = if_exists(command, '|');
        int bg_flag = if_exists(command, '&');

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

        // an to command exei ampersant sto telos, vazw sti thesi tou & NULL gia na ektelestei kanonika i entoli.
        int i = 0;
        while (token_array[i] != NULL)
        {
            if (!strcmp(token_array[i], "&"))
            {
                token_array[i] = NULL;
            }
            i++;
        }

        // for (int i = 0; i < count; i++)
        // {
        //     fprintf(stdout, "%s ", token_array[i]);
        // }
        // fprintf(stdout, "\n");

        if (!strcmp(token_array[0], "exit")) // exit shell
        {
            printf("logout\n");
            fflush(stdout);
            break;
        }

        if (!redirect_input_flag && !redirect_output_flag && !pipe_flag) // executing simple commands
        {
            execute_simple_command(token_array, bg_flag);
        }

        else if (redirect_input_flag || redirect_output_flag || !pipe_flag) // handling redirections ONLY
        {
            handle_redirections(token_array, bg_flag);
        }

        else if (pipe_flag)
        {
            handle_pipes(token_array, num_pipes, count);
        }

        free(input);
        // end of while loop
    }

    return 0;
}