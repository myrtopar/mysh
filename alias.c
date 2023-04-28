#include "functions.h"

aliasnode *search_alias(aliasnode **alias_array, char *alias)
{
    for (int i = 0; i < 30; i++)
    {
        if (alias_array[i] != NULL)
        {
            if (!strcmp(alias_array[i]->alias, alias))
            {
                return alias_array[i];
            }
        }
    }

    return NULL;
}

int createalias(aliasnode **alias_array, char *token_array[])
{
    if (search_alias(alias_array, token_array[1]) != NULL) // yparxei idi kapoio command me auto to alias
        return 0;

    aliasnode *new_alias = (aliasnode *)malloc(sizeof(struct aliasnode));
    strcpy(new_alias->alias, token_array[1]);

    char *command = (char *)malloc(100 * sizeof(char));

    int i = 2;
    while (token_array[i] != NULL)
    {
        command = strcat(command, token_array[i]);
        command = strcat(command, " ");
        i++;
    }
    strcpy(new_alias->command, command);
    free(command);

    int j = 0;
    while (alias_array[j] != NULL)
        j++;

    // placing the struct in the first available position in the array of aliases
    alias_array[j] = new_alias;

    return 1;
}

void destroyalias(aliasnode **alias_array, char *alias)
{
    for (int i = 0; i < 30; i++)
    {
        if (alias_array[i] != NULL)
        {
            if (!strcmp(alias_array[i]->alias, alias))
            {
                free(alias_array[i]);
                alias_array[i] == NULL;
                return;
            }
        }
    }
    return;
}