#include "functions.h"

void interrupt_handlerr(int signum)
{
    // exiting child process...
    printf("about to terminate...\n");
    exit(130);
}

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

    char command[MAX_CHAR_INPUT];
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
        printf(COLOR_BRIGHT_GREEN BOLD "mysh> " COLOR_RESET);
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
        int wildcard_flag = if_exists(command, '*') || if_exists(command, '?');

        char *input = add_whitespaces(command, occ);

        // copy to command sto history
        int index = nth_command % 20;
        strcpy(history_array[index], input);
        nth_command++;

        char *input_dup = parse_command(input, token_array);
        int count = token_count(token_array);

        if (count == 0) // if there are no tokens in the line, continue to fgets again
        {
            free(input);
            free(input_dup);
            continue;
        }

        // Checking for history command
        if (!strcmp(token_array[0], "myhistory") && token_array[1] == NULL)
        {
            show_history(history_array, nth_command);
            free(input);
            free(input_dup);
            continue;
        }

        else if (!strcmp(token_array[0], "myhistory") && isDigit(token_array[1]))
        {
            // ektelei tin n-osti entoli APO TO TELOS!
            int num = atoi(token_array[1]);
            if (num > 20 || num == 0)
            {
                printf("Invalid number\n");
                free(input);
                free(input_dup);
                continue;
            }
            char *nth = history_array[index - num];
            parse_command(nth, token_array);
            count = token_count(token_array);
        }

        // Checking for exit command
        if (!strcmp(token_array[0], "exit")) // exit shell
        {
            free(input);
            free(input_dup);
            printf("logout\n");
            fflush(stdout);
            break;
        }

        // Checking for cd command
        if (!strcmp(token_array[0], "cd"))
        {
            change_dir(token_array[1]);
            free(input);
            free(input_dup);
            continue;
        }

        // Checking for alias commands
        if (!strcmp(token_array[0], "createalias"))
        {
            if (createalias(alias_array, token_array) == 0)
                printf("Error creating alias. This alias already exists.\n");

            free(input);
            free(input_dup);
            continue;
        }
        else if (!strcmp(token_array[0], "destroyalias"))
        {
            destroyalias(alias_array, token_array[1]);
            free(input);
            free(input_dup);
            continue;
        }

        // searching if the command is an alias to some unix command defined in the past
        aliasnode *alias = search_alias(alias_array, token_array[0]);
        if (alias != NULL)
        {
            redirect_output_flag = if_exists(alias->command, '>');
            redirect_input_flag = if_exists(alias->command, '<');
            pipe_flag = if_exists(alias->command, '|');
            bg_flag = if_exists(alias->command, '&');

            parse_command(alias->command, token_array); // parse the actual unix command that corresponds to the alias
            count = token_count(token_array);
        }

        if (wildcard_flag)
        {
            expand_wildcard(token_array, redirect_input_flag, redirect_output_flag, pipe_flag, num_pipes, bg_flag);
            free(input);
            free(input_dup);
            continue;
        }

        if (!redirect_input_flag && !redirect_output_flag && !pipe_flag) // executing simple commands
        {
            execute_simple_command(token_array, bg_flag);
        }

        else if ((redirect_input_flag || redirect_output_flag) && !pipe_flag) // handling redirections ONLY
        {
            handle_redirections(token_array, bg_flag);
        }

        else if (pipe_flag)
        {
            handle_pipes(token_array, num_pipes, count, bg_flag, redirect_output_flag);
        }

        free(input);
        free(input_dup);
        //    end of while loop
    }

    for (int i = 0; i < 20; i++)
        free(history_array[i]);

    free(history_array);

    return 0;
}
