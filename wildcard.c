#include "functions.h"

void expand_asterisk(char **token_array, int in_redirect, int out_redirect, int pipe, int bg)
{
    printf("in expand asterisk\n");
    char *expanded_token_array[MAX_ARGS];
    int i = 0, j = 0;
    glob_t files;
    int res;

    while (token_array[i] != NULL)
    {
        if (if_exists(token_array[i], '*'))
        {
            if ((res = glob(token_array[i], 0, NULL, &files)) == 0)
            {
                printf("token: %s,\n", token_array[i]);
                for (size_t k = 0; k < files.gl_pathc; k++)
                {
                    printf("%s, %d\n", files.gl_pathv[k], j);
                    expanded_token_array[j++] = files.gl_pathv[k];
                }
            }
        }
        else
        {
            printf("%s, %d\n", token_array[i], j);
            expanded_token_array[j++] = token_array[i];
        }
        i++;
    }
    expanded_token_array[j] = NULL;

    // int m = 0;
    // while (expanded_token_array[m] != NULL)
    //     printf("%s\n", expanded_token_array[m++]);

    globfree(&files);
}