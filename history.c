#include "functions.h"

void show_history(char **history, int nth)
{
    for (int i = 0; i < 20; i++)
    {
        fprintf(stdout, "%s\n", history[i]);
        if (i > nth - 1)
            break;
    }
}