#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "tokenizer.h"

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_cwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_cwd, "cwd", "show current working directory"},
  {cmd_cd, "cd", "set cwd to a new path"},
};

/* Prints a helpful description for the given command */
int cmd_help(struct tokens *tokens) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(struct tokens *tokens) {
  exit(0);
}

/* Show current work directory path */
int cmd_cwd(struct tokens *tokens) {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL)
    printf("%s\n", cwd);
  else
    perror("getcwd() error");
  return 0;
}

/* Change current work directory */
int cmd_cd(struct tokens *tokens) {
  char *dir = tokens_get_token(tokens, 1);
  if (chdir(dir) == 0) 
    printf("change to directory %s\n", dir);
  else
    perror("chdir() error");  
  return 0;
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

/* redirect the input or output of command */
void redirect(char *direct, char* file) {
  int newfd;

  if (*direct == '>') {
    if ((newfd = open(file,  O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
      perror(file);
      exit(1);
    }
    dup2(newfd, 1);
  }
  if (*direct == '<') {
    if ((newfd = open(file, O_RDONLY)) < 0) {
      perror(file);
      exit(1);
    }
    dup2(newfd, 0);
  }
}

/* execute programs */
void execute(struct tokens *tokens) {
  /* environment PATH */
  char *path_env[] = {"/usr/local/sbin", "/usr/local/bin", 
        "/usr/sbin", "/usr/bin", "/sbin", "/bin", ".", 0};
  int path_len = 7; /* number of items in path_env ! */
  /* put tokenized result into an array */
  int len = tokens_get_length(tokens);
  char *cmd_arg[len+1];
  int i;
  int cut = len; /* the place where redirect happens, default is no redirection */
  for (i = 0; i < len; i++) {
    cmd_arg[i] = tokens_get_token(tokens, i);
    if (*cmd_arg[i] == '<' || *cmd_arg[i] == '>') {
      printf("redirect to file '%s' using '%s'\n", tokens_get_token(tokens, i+1), cmd_arg[i]);
      cut = i;
      redirect(cmd_arg[i], tokens_get_token(tokens, i+1));
    }
  }
  for (i = 0; i < cut; i++)
    cmd_arg[i] = tokens_get_token(tokens, i);
  cmd_arg[cut] = 0;

  int cmd_len = (int)strlen(cmd_arg[0]);
  /* loop over all paths in environment variable PATH */
  for (i = 0; i < path_len; i++) {
    /* first concatate env_path and type in command */
    int env_len = (int)strlen(path_env[i]);
    char str2[cmd_len + 2]; /* one for '/0', one for '/' */
    strcpy(str2, "/");
    strcat(str2, cmd_arg[0]);
    char prog_path[cmd_len + env_len + 2]; /* same as above */
    strcpy(prog_path, path_env[i]);
    strcat(prog_path, str2);
    /* then execute the progam at prog_path */
    // printf("%s is now the path to execute", prog_path);

    execv(prog_path, cmd_arg);
    // printf("%s is not the place of this program", prog_path);
  }
  /* after looping over all paths, print error message and exit */
  perror("Please check the command you typed ");
  exit(0);  /* exit is important, if not, current shell will be of the child process */
}

  
int main(int argc, char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else { /* execute program */
      int pid;
      pid = fork();
      if (pid == 0) { /* child process */
        /* run commands as programs. */
        signal(SIGINT, SIG_DFL); /* Ctrl-c stops current program */
        execute(tokens);
      }
      else { /* parent process */
        signal(SIGINT, SIG_IGN); /* ingnore in parent process to only stop child process */
        int status;
        wait(&status);
        // printf("%d\n", status);
      }
    }

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
