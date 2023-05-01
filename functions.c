#include "functions.h"

#define READ 0
#define WRITE 1

void interrupt_handler(int signum)
{
    // exiting child process...
    printf("about to terminate...\n");
    exit(130);
}

void suspend_handler(int signum)
{
    // suspending child process...
    signal(SIGTSTP, SIG_DFL);
    pid_t pgid = getpgrp();
    printf("about to suspend...\n");
    killpg(pgid, SIGSTOP); // Suspend the child process group
}

struct sigaction act = {
    .sa_handler = interrupt_handler,
    .sa_flags = 0};

int occurences(char *str, int n)
{
    int count = 0;
    while (*str != '\0')
    {
        if (*str == n)
            count++;
        str++;
    }

    return count;
}

char *add_whitespaces(char *str, int occur)
{
    char *with_whitespaces = malloc(strlen(str) + (2 * occur) + 1);
    char *init = str;

    int index = 0;
    while (*str != '\0')
    {
        if (*str == '|' || *str == '<')
        {
            with_whitespaces[index++] = ' ';
            with_whitespaces[index++] = *str;
            with_whitespaces[index++] = ' ';
        }
        else if (*str == '>' && *(str + 1) != '>' && *(str - 1) != '>')
        {
            with_whitespaces[index++] = ' ';
            with_whitespaces[index++] = *str;
            with_whitespaces[index++] = ' ';
        }

        // subcases for append operator >> so that whitespaces can wrap around >> operator
        else if (*str == '>' && *(str + 1) == '>')
        {
            with_whitespaces[index++] = ' ';
            with_whitespaces[index++] = *str;
        }
        else if (*str == '>' && *(str - 1) == '>' && *(str + 1) != '>')
        {
            with_whitespaces[index++] = *str;
            with_whitespaces[index++] = ' ';
        }
        else if (*str == '&' || *str == '"')
        {
            with_whitespaces[index++] = ' ';
        }
        else
            with_whitespaces[index++] = *str;

        str++;
    }
    with_whitespaces[index] = '\0';
    return with_whitespaces;
}

int if_exists(char *str, int n)
{
    while (*str != '\0')
    {
        if (*str == n)
            return 1;
        str++;
    }
    return 0;
}

void parse_command(char *input, char *token_array[])
{
    // to token_array exei ta tokens tou input spasmena: px gia entolh input ls -al: token_array[0] = ls, token_array[1] = -al

    int count = 0;
    char *input_dup = strdup(input);

    char *token = strtok(input_dup, " \t");
    while (token != NULL)
    {
        token_array[count] = token;
        count++;
        token = strtok(NULL, " \t");
    }

    token_array[count] = NULL;
    return;
}

void execute_simple_command(char **token_array, int background)
{

    pid_t pid, bg_pgid, fg_pgid;
    int status, exit_status;

    if ((pid = fork()) < 0)
        perror("fork failed");

    if (pid == 0) // child process
    {
        if (background == 1) // an to process einai gia background, to vazw se ena neo process group me leader ton eauto tou kai den kanw kati allo
        {
            bg_pgid = getpid();
            setpgid(bg_pgid, bg_pgid);
        }
        else if (background == 0)
        {
            // an to process einai gia foreground, klironomei to process group tou shell. Tou dinw acess sto terminal gia na mporei na doulepsei
            signal(SIGINT, SIG_DFL);          // handling sigint signals (ctrl-c)
            signal(SIGTSTP, suspend_handler); // handling sigtstp signals (ctrl-z)
            fg_pgid = getpid();
            setpgid(fg_pgid, fg_pgid);
            tcsetpgrp(STDIN_FILENO, fg_pgid);
        }

        execvp(token_array[0], token_array);

        perror("Error executing command");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) // parent process
    {
        if (!background) // to parent process perimenei mono ta foreground children processes. Ta ypoloipa ta afinei na trexoun sto background
        {
            int waitval = waitpid(-1, &status, 0);
        }
        // else if (background)
        // {
        //     printf("Done              ");
        //     for (int i = 0; token_array[i] != NULL; i++)
        //     {
        //         fprintf(stdout, "%s ", token_array[i]);
        //     }
        //     fprintf(stdout, "\n");
        // }

        pid_t shell_pgid = getpid();
        tcsetpgrp(STDIN_FILENO, shell_pgid); // to shell pairnei pali ton elegxo tou controlling terminal kai synexizei na diavazei input
    }
}

void handle_redirections(char **token_array, int background)
{
    int fd_in, fd_out;
    pid_t pid, bg_pgid, fg_pgid;
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

                if (background) // an to process einai gia background, to vazw se ena neo process group me leader ton eauto tou kai den kanw kati allo
                {
                    bg_pgid = getpid();
                    setpgid(bg_pgid, bg_pgid);
                }
                else
                {                                  // an to process einai gia foreground, klironomei to process group tou shell. Tou dinw acess sto terminal gia na mporei na doulepsei
                    sigaction(SIGINT, &act, NULL); // handling sigint signals
                    fg_pgid = getpid();
                    setpgid(fg_pgid, fg_pgid);
                    tcsetpgrp(STDIN_FILENO, fg_pgid);
                }

                dup2(fd_out, STDOUT_FILENO); // redirect stdout to the file opened for output
                execvp(token_array[0], token_array);

                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
            else if (pid > 0) // parent process
            {
                if (!background) // to parent process perimenei mono ta foreground children processes. Ta ypoloipa ta afinei na trexoun sto background
                {
                    int waitval = waitpid(-1, &status, 0);
                }

                dup2(STDOUT_FILENO, fd_out); // restore the STDOUT_FILENO fd
                if (STDOUT_FILENO != fd_out)
                    close(fd_out);

                pid_t shell_pgid = getpid();
                tcsetpgrp(STDIN_FILENO, shell_pgid); // to shell pairnei pali ton elegxo tou controlling terminal kai synexizei na diavazei input

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

                if (background) // an to process einai gia background, to vazw se ena neo process group me leader ton eauto tou kai den kanw kati allo
                {
                    bg_pgid = getpid();
                    setpgid(bg_pgid, bg_pgid);
                }
                else
                {                                  // an to process einai gia foreground, klironomei to process group tou shell. Tou dinw acess sto terminal gia na mporei na doulepsei
                    sigaction(SIGINT, &act, NULL); // handling sigint signals
                    fg_pgid = getpid();
                    setpgid(fg_pgid, fg_pgid);
                    tcsetpgrp(STDIN_FILENO, fg_pgid);
                }

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

                execvp(token_array[0], token_array);
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
            else if (pid > 0) // parent process
            {
                if (!background) // to parent process perimenei mono ta foreground children processes. Ta ypoloipa ta afinei na trexoun sto background
                {
                    int waitval = waitpid(-1, &status, 0);
                }

                dup2(STDIN_FILENO, fd_in); // restore the STDIN_FILENO fd
                if (STDIN_FILENO != fd_in)
                    close(fd_in);

                pid_t shell_pgid = getpid();
                tcsetpgrp(STDIN_FILENO, shell_pgid); // to shell pairnei pali ton elegxo tou controlling terminal kai synexizei na diavazei input

                break;
            }
        }
    }
    return;
}

void handle_pipes(char **token_array, int num_pipes, int token_count, int background, int redirect)
{
    int fds[num_pipes][2]; // 2d array, kathe grammi einai kai ena pipe, pou exei 2 ints, to write fd kai to read fd
    pid_t pid, bg_pgid, fg_pgid;
    int status;
    null_delim(token_array);

    // Creating all the pipes needed
    for (int i = 0; i < num_pipes; i++)
    {
        if (pipe(fds[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    pid_t leader_fgid, leader_bgid;
    // Forks. Kathe paidi tha klironomisei ola ta parapanw pipes
    for (int i = 0; i <= num_pipes; i++) //<= num_pipes giati ta processes einai kata 1 parapanw apo ta pipes
    {
        if ((pid = fork()) < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        { // child process. Tha kanw ta aparaitita dup2 kai meta exec

            if (background)
            {
                if (i == 0)
                { // ean einai to prwto process, tha ftiaksei ena process group kai tha thesei ton eauto tou ws leader process
                    bg_pgid = getpid();
                    setpgid(bg_pgid, bg_pgid);
                }
                else if (i > 0)
                { // ean den einai to prwto process tou pipeline, tote apla to vazw sto process group tou
                    setpgid(getpid(), leader_bgid);
                }
            }
            else if (!background)
            {
                if (i == 0)
                { // ean einai to prwto process, tha ftiaksei ena process group kai tha thesei ton eauto tou ws leader process
                    fg_pgid = getpid();
                    setpgid(fg_pgid, fg_pgid);
                    tcsetpgrp(STDIN_FILENO, fg_pgid);
                }
                else if (i > 0)
                { // ean den einai to prwto process tou pipeline, tote apla to vazw sto process group tou
                    setpgid(getpid(), leader_fgid);
                    tcsetpgrp(STDIN_FILENO, leader_fgid);
                }
            }

            if (i == 0) // stin prwti diergasia arkei na anakateuthynw mono to output. Den exw na parw input apo kapou
                dup2(fds[0][WRITE], STDOUT_FILENO);

            else if (i == num_pipes) // antistoixa gia tin teleutaia diergasia, anakateuthynw mono to input tou proigoumenou apo to pipe. To output de me noiazei paei kanonika stdout
            {
                dup2(fds[i - 1][READ], STDIN_FILENO);

                if (redirect == 1)
                {
                    int fd_out;
                    int redir_index = redirect_index(token_array, token_count);

                    if (!strcmp(token_array[redir_index], ">"))
                        fd_out = open(token_array[redir_index + 1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    else if (!strcmp(token_array[redir_index], ">>"))
                        fd_out = open(token_array[redir_index + 1], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

                    token_array[redir_index] = NULL;

                    dup2(fd_out, STDOUT_FILENO);
                }
            }
            else
            { // diaforetika enwnw kai ta 2 akra tou pipe me to process
                dup2(fds[i][WRITE], STDOUT_FILENO);
                dup2(fds[i - 1][READ], STDIN_FILENO);
            }

            // afou ekana ola ta dup2, kleinw ola ta pipe ends
            for (int j = 0; j < num_pipes; j++)
            {
                close(fds[j][READ]);
                close(fds[j][WRITE]);
            }

            int index = command_to_exec(token_array, i, token_count);
            execvp(token_array[index], &token_array[index]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        if (i == 0)
        {
            leader_bgid = pid; // to PRWTO pid sto parent process einai to process leader tou process group
            leader_fgid = pid;
        }
    }

    // kleinw ola ta pipe ends sto parent process
    for (int i = 0; i < num_pipes; i++)
    {
        close(fds[i][READ]);
        close(fds[i][WRITE]);
    }

    if (!background) // to parent process perimenei mono ta foreground children processes. Ta ypoloipa ta afinei na trexoun sto background
    {
        for (int i = 0; i <= num_pipes; i++)
        {
            int waitval = waitpid(-1, &status, 0);
        }
    }

    pid_t shell_pgid = getpid();
    tcsetpgrp(STDIN_FILENO, shell_pgid); // to shell pairnei pali ton elegxo tou controlling terminal kai synexizei na diavazei input
}

int redirect_index(char **token_array, int token_count)
{
    for (int i = 0; i < token_count; i++)
    {
        if (token_array[i] != NULL && (!strcmp(token_array[i], ">") || !strcmp(token_array[i], ">>")))
            return i;
    }

    return -1;
}

int command_to_exec(char **token_array, int nth_command, int token_count)
{
    // epistrefei to index tou stoixeioy pou ksekinaei to command gia to exec
    // no of command == how many nulls i must encounter
    int null_count = 0;
    for (int i = 0; i < token_count; i++)
    {
        if (null_count == nth_command)
            return i;
        if (token_array[i] == NULL)
            null_count++;
    }
    return 0;
}

void null_delim(char **array)
{
    // pairnei to token array kai vazei null opou exei | kai etsi diaxwrizei tis entoles metaksy tous gia to exec meta
    int i = 0;
    while (array[i] != NULL)
    {
        if (!strcmp(array[i], "|"))
        {
            array[i] = NULL;
        }
        i++;
    }
}

int isDigit(char *str)
{
    int i = 0;
    for (; str[i] != '\0'; i++)
    {
        if (!isdigit(str[i]))
            return 0;
    }

    return 1;
}

int token_count(char *token_array[])
{
    int count = 0;
    while (token_array[count] != NULL)
        count++;

    return count;
}

void change_dir(char *path)
{
    int res;

    if (path == NULL || !strcmp(path, "~"))
        res = chdir(getenv("HOME"));

    else
        res = chdir(path);

    if (res)
        printf("Failed to change directory.\n");

    return;
}
