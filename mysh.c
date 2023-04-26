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

#define COLOR_RED "\x1b[95m"
#define COLOR_RESET "\x1b[0m"
#define BOLD "\x1b[1m"

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

    char command[MAX_CHAR_INPUT]; //= malloc(MAX_CHAR_INPUT);
    char *token_array[MAX_ARGS];
    aliasnode *alias_array[30]; // static array of 30 aliasptr pointers
    for (int i = 0; i < 30; i++)
    {
        alias_array[i] = NULL;
    }

    int nth_command = 0; // n-osth entolh sto mysh
    char **history_array = (char **)malloc(20 * sizeof(char *));
    for (int i = 0; i < 20; i++)
    {
        history_array[i] = (char *)malloc(MAX_CHAR_INPUT * sizeof(char));
    }

    while (1)
    {
        printf(COLOR_RED BOLD "mysh> " COLOR_RESET);
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

        // copy to command sto history
        int index = nth_command % 20;
        strcpy(history_array[index], input);
        nth_command++;

        parse_command(input, token_array);
        int count = token_count(token_array);

        if (count == 0) // αν η γραμμη δεν εχει κανενα token, προχώρα
            continue;

        if (!strcmp(token_array[0], "myhistory") && token_array[1] == NULL)
        {
            show_history(history_array, nth_command);
            continue;
        }

        else if (!strcmp(token_array[0], "myhistory") && isDigit(token_array[1]))
        {
            // ektelei tin n-osti entoli APO TO TELOS!
            int num = atoi(token_array[1]);
            if (num > 20 || num == 0)
            {
                printf("Invalid number\n");
                continue;
            }
            char *nth = history_array[index - num];
            parse_command(nth, token_array);
            count = token_count(token_array);
        }
        // for (int i = 0; i < count; i++)
        // {
        //     printf("%s\n", token_array[i]);
        // }

        // for (int i = 0; i < 30; i++)
        // {
        //     if (alias_array[i] != NULL)
        //         printf("%d alias: %s, command: %s\n", i, alias_array[i]->alias, alias_array[i]->command);
        // }

        if (!strcmp(token_array[0], "createalias"))
        {
            if (createalias(alias_array, token_array) == 0)
                printf("Error creating alias. This alias already exists.\n");
            continue;
        }

        if (!strcmp(token_array[0], "exit")) // exit shell
        {
            printf("logout\n");
            fflush(stdout);
            break;
        }
        aliasnode *alias = search_alias(alias_array, token_array[0]);
        if (alias != NULL)
        {
            redirect_output_flag = if_exists(alias->command, '>');
            redirect_input_flag = if_exists(alias->command, '<');
            pipe_flag = if_exists(alias->command, '|');
            bg_flag = if_exists(alias->command, '&');

            parse_command(alias->command, token_array);
            count = token_count(token_array);
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
            handle_pipes(token_array, num_pipes, count, bg_flag);
        }

        free(input);
        // end of while loop
    }

    for (int i = 0; i < 20; i++)
        free(history_array[i]);

    free(history_array);

    return 0;
}