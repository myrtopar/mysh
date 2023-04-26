// struct που περιεχει πληροφοριες για ενα alias kai to command που αντιπροσωπευει
typedef struct aliasnode aliasnode;
struct aliasnode
{
    char alias[100];
    char command[100];
};

int occurences(char *, int);
char *add_whitespaces(char *, int);
int if_exists(char *, int);
void parse_command(char *, char **);
void execute_simple_command(char **, int);
void handle_redirections(char **, int);
void handle_pipes(char **, int, int, int);
void null_delim(char **);
int command_to_exec(char **, int, int);
void show_history(char **, int);
int isDigit(char *);
int token_count(char **);
int createalias(aliasnode **, char **);
aliasnode *search_alias(aliasnode **, char *);
