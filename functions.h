#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <glob.h>
#include <ctype.h>

#define MAX_CHAR_INPUT 300
#define MAX_ARGS 50

#define COLOR_BRIGHT_GREEN "\x1b[92m"

#define COLOR_RESET "\x1b[0m"
#define BOLD "\x1b[1m"

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
void handle_pipes(char **, int, int, int, int);
void null_delim(char **);
int command_to_exec(char **, int, int);
void show_history(char **, int);
int isDigit(char *);
int token_count(char **);
int createalias(aliasnode **, char **);
aliasnode *search_alias(aliasnode **, char *);
void destroyalias(aliasnode **, char *);
void expand_asterisk(char **, int, int, int, int, int);
int redirect_index(char **, int);
void change_dir(char *);
