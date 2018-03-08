/****************************************************************
 * Name        : Eric Groom                                     *
 * Class       : CSC 415                                        *
 * Date        : 3/12/2018                                      *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple commands. The main   *
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

int strip_nl(char *str, int size)
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
    int index = 0;
    int command = 0;
    char *saveptr1, *saveptr2;
    for (char *token = strtok_r(raw, "|", &saveptr1); token != NULL; token = strtok_r(NULL, "|", &saveptr1))
    {
        for (char *subtoken = strtok_r(token, " ", &saveptr2); subtoken != NULL; subtoken = strtok_r(NULL, " ", &saveptr2))
        {
            argv[index] = subtoken;
            argc[command]++;
            index++;
        }
        command++;
        argv[++index] = NULL;
    }
}

void exec_children(char **argv, int *argc)
{
    int pd[2];
    pipe(pd);
    int *hasPipe = argc[1] > 0;
    pid_t child_pid = -1;
    if (strncmp(argv[0], "exit", 4) == 0)
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

        execvp(*argv, argv);
        perror("error executing");
    }
    else
    {
        if (hasPipe && fork() == 0)
        { // child 2
            dup2(pd[0], STDIN_FILENO);
            close(pd[0]);
            close(pd[1]);
            
            int index = argc[0]+1;
            execvp(argv[index], &argv[index]);
            perror("error executing");
        }
        else
        { // parent
            close(pd[0]);
            close(pd[1]);
            wait(NULL); // wait for child
        }
    }
}

int first_ws(char *str) {
    int len = strlen(str);
    for(int i = 0; i < len; i++) {
        if (str[i] == '\n' || str[i] == '\t' || str[i] == ' ' ) {
            return i;
        }
    }
    return len;
}

void start_loop()
{
    char buf[BUFFERSIZE];
    int *myargc = calloc(PIPECNTMAX, sizeof(int));
    char **myargv = calloc(ARGVMAX, sizeof(char *));
    print_prompt();
    while (fgets(buf, BUFFERSIZE, stdin))
    {
        int len = strip_nl(buf, BUFFERSIZE);
        parse(buf, myargc, myargv);
        if (*myargv != NULL && first_ws(*myargv) > 0) {
            exec_children(myargv, myargc);
        }
        memset(myargv, 0, ARGVMAX);
        memset(myargc, 0, ARGVMAX);
        print_prompt();
    }
    printf("\n");
    // free(myargc);
    // for(int i = 0; i < ARGVMAX; i++) {
    //     free(myargv[i]);
    // }
    // free(myargv);
}

int main(int argc, char **argv)
{
    start_loop();
    return 0;
}
