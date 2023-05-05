#include "functions.h"

void show_history(char **history, int nth)
{
    if (nth > 20)
    {
        int index = nth % 20;
        for (int i = index; i < 20; i++)
            printf("%s\n", history[i]);

        for (int i = 0; i < index; i++)
            printf("%s\n", history[i]);
    }

    else
    {
        for (int i = 0; i < 20; i++)
        {
            printf("%s\n", history[i]);
            if (i > nth - 1)
                break;
        }
    }
}