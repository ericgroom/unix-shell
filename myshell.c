/****************************************************************
 * Name        : Eric Groom                                     *
 * Class       : CSC 415                                        *
 * Date  	   : TODO                                           *
 * Description :  Writting a simple bash shell program          *
 * 	        	  that will execute simple commands. The main   *
 *                goal of the assignment is working with        *
 *                fork, pipes and exec system calls.            *
 ****************************************************************/

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

void exec_child(char **argv, int argc)
{
    pid_t child_pid = -1;
    if (strncmp(argv[0], "exit", 4) == 0) {
        exit(0);
    }
    child_pid = fork();
    if (child_pid < 0)
    {
        perror("forking error");
    }
    else if (child_pid == 0)
    {
        int exec_return = -1;
        exec_return = execvp(argv[0], argv);
        if (exec_return < 0)
            perror("error executing");
    }
    else
    {
        wait(NULL); // wait for child
    }
}

void exec_children(char **argv, int *argc, char **argv_buf)
{
    // int no_commands = 0;
    // int pd[2];
    // while(argc[no_commands] != NULL) no_commands++;
    for (int i = 0; argv[i] != NULL; i++)
    {
        argv_buf[i] = malloc(strlen(argv[i]) + 1);
        strcpy(argv_buf[i], argv[i]);
    }
    argv_buf[4] = NULL;
    exec_child(argv_buf, argc[0]);
}

void start_loop()
{
    char buf[BUFFERSIZE];
    int *myargc = calloc(PIPECNTMAX, sizeof(int));
    char **myargv = calloc(PIPECNTMAX * MAX_ARGV_PER_COMMAND, sizeof(char *));
    char **callingargv = calloc(5, sizeof(char *));
    print_prompt();
    while (fgets(buf, BUFFERSIZE, stdin))
    {
        int len = strip(buf, BUFFERSIZE);
        parse(buf, myargc, myargv);
        exec_children(myargv, myargc, callingargv);
        memset(myargv, 0, ARGVMAX);
        memset(myargc, 0, ARGVMAX);
        print_prompt();
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    start_loop();
    return 0;
}
