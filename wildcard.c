#include "functions.h"

void expand_asterisk(char **token_array, int in_redirect, int out_redirect, int pipe, int num_pipes, int bg)
{
    char *expanded_token_array[MAX_ARGS];
    int i = 0, j = 0;
    glob_t files;
    int res;

    while (token_array[i] != NULL)
    {
        if (if_exists(token_array[i], '*') || if_exists(token_array[i], '?'))
        {
            if ((res = glob(token_array[i], 0, NULL, &files)) == 0)
            {
                for (size_t k = 0; k < files.gl_pathc; k++)
                {
                    expanded_token_array[j++] = files.gl_pathv[k];
                }
            }
        }
        else
        {
            expanded_token_array[j++] = token_array[i];
        }
        i++;
    }

    expanded_token_array[j] = NULL;

    // int m = 0;
    // while (expanded_token_array[m] != NULL)
    //     printf("%s\n", expanded_token_array[m++]);
    int count = token_count(expanded_token_array);

    if (!in_redirect && !out_redirect && !pipe) // executing simple commands
    {
        execute_simple_command(expanded_token_array, bg);
    }

    else if (in_redirect || out_redirect || !pipe) // handling redirections ONLY
    {
        handle_redirections(expanded_token_array, bg);
    }

    else if (pipe)
    {
        handle_pipes(expanded_token_array, num_pipes, count, bg, out_redirect);
    }

    globfree(&files);
}