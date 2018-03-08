/****************************************************************
 * Name        : Eric Groom                                     *
 * Class       : CSC 415                                        *
 * Date  	   : TODO                                           *
 * Description :  Writting a simple bash shell program          *
 * 	        	  that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/
// TODO fix tabs in header
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFERSIZE 256
#define PROMPT "myShell >> "
#define PROMPTSIZE sizeof(PROMPT)
#define ARGVMAX 64
#define PIPECNTMAX 10
#define MAX_ARGV_PER_COMMAND 4

void print_prompt()
{
    printf("%s", PROMPT);
}

int strip(char *str, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (str[i] == '\n')
        {
            str[i] = 0;
            return i;
        }
    }
    return size + 1;
}

void parse(char *raw, int *argc, char **argv)
{
    int command = 0;
    char *saveptr1, *saveptr2;
    for (char *token = strtok_r(raw, "|", &saveptr1); token != NULL; token = strtok_r(NULL, "|", &saveptr1))
    {
        for (char *subtoken = strtok_r(token, " ", &saveptr2); subtoken != NULL; subtoken = strtok_r(NULL, " ", &saveptr2))
        {
            int index = argc[command] + command * MAX_ARGV_PER_COMMAND;
            argv[index] = subtoken;
            argc[command]++;
        }
        command++;
    }
}

void exec_children(char **argv, int *argc)
{
    // int no_commands = 0;
    int pd[2];
    pipe(pd);
    // while(argc[no_commands] != NULL) no_commands++;
    char **command1_argv = calloc(5, sizeof(char *));
    char **command2_argv = calloc(5, sizeof(char *));
    for (int i = 0; argv[i] != NULL; i++)
    {
        command1_argv[i] = malloc(strlen(argv[i]) + 1);
        strcpy(command1_argv[i], argv[i]);
    }
    for (int i = 0; argv[i + 4] != NULL; i++)
    {
        command2_argv[i] = malloc(strlen(argv[i + 4]) + 1);
        strcpy(command2_argv[i], argv[i + 4]);
    }
    command1_argv[4] = NULL;
    command2_argv[4] = NULL;

    char hasPipe = command2_argv[0];

    pid_t child_pid = -1;
    if (strncmp(command1_argv[0], "exit", 4) == 0)
    {
        exit(0);
    }
    child_pid = fork();
    if (child_pid < 0)
    {
        perror("forking error");
    }
    else if (child_pid == 0) // child one
    {
        if (hasPipe) {
        dup2(pd[1], STDOUT_FILENO);
        }

        close(pd[0]);
        close(pd[1]);

        execvp(command1_argv[0], command1_argv);
        perror("error executing");
    }
    else
    {
        if (hasPipe && fork() == 0)
        { // child 2
            dup2(pd[0], STDIN_FILENO);
            close(pd[0]);
            close(pd[1]);

            execvp(command2_argv[0], command2_argv);
            perror("error executing");
        }
        else
        { // parent
            close(pd[0]);
            close(pd[1]);
            wait(NULL); // wait for child
        }
    }
    for (int i = 0; i < 5; i++) {
        free(command1_argv[i]);
        free(command2_argv[i]);
    }
    free(command1_argv);
    free(command2_argv);
}

void start_loop()
{
    char buf[BUFFERSIZE];
    int *myargc = calloc(PIPECNTMAX, sizeof(int));
    char **myargv = calloc(PIPECNTMAX * MAX_ARGV_PER_COMMAND, sizeof(char *));
    print_prompt();
    while (fgets(buf, BUFFERSIZE, stdin))
    {
        int len = strip(buf, BUFFERSIZE);
        parse(buf, myargc, myargv);
        exec_children(myargv, myargc);
        memset(myargv, 0, ARGVMAX);
        memset(myargc, 0, ARGVMAX);
        print_prompt();
    }
    printf("\n");
    free(myargc);
    for(int i = 0; i < ARGVMAX; i++) {
        free(myargv[i]);
    }
    free(myargv);
}

int main(int argc, char **argv)
{
    start_loop();
    return 0;
}
