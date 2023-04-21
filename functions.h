int occurences(char *, int);
char *add_whitespaces(char *, int);
int if_exists(char *, int);
void execute_simple_command(char **, int);
void handle_redirections(char **);
void handle_pipes(char **, int, int);
void null_delim(char **);
int command_to_exec(char **, int, int);
