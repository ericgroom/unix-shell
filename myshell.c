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
#define PROMPTNAME "myShell "
#define PROMPTSEPARATOR " >> "
#define ARGVMAX 64
#define PIPECNTMAX 10
#define MAX_ARGV_PER_COMMAND 4

void print_prompt()
{
    char dir[256];
    getcwd(dir, 256);
    printf("%s%s%s", PROMPTNAME, dir, PROMPTSEPARATOR);
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
    // no of commands
    int commands = 0;
    while(commands < PIPECNTMAX+1 && argc[commands] > 0) commands++;
    printf("no of commands: %d\n", commands);

    // create pipes
    int pd[PIPECNTMAX*2];
    for(int i = 0; i < PIPECNTMAX; i++) {
        pipe(&pd[i*2]);
    }
    
    // determine start index for each command
    int argv_i[PIPECNTMAX+1];
    argv_i[0] = 0;
    printf("argv_i -- ");
    for(int i = 1; i < commands; i++) {
        argv_i[i] = argv_i[i-1] + argc[i-1] + 1;
        printf("%d,", argv_i[i]);
    }
    printf("END\n");
    for(int i = 0; i < commands; i++) {
        printf("i: %d\n", i);
        int w_index = (i)*2+1;
        int r_index = (i-1)*2;
        printf("w_index: %d\n", w_index);
        printf("r_index: %d\n", r_index);
    }
    // fork loop
    for(int i = 0; i < commands; i++) {
        pid_t child_pid = -1;
        child_pid = fork();
        if (child_pid < 0) {
            perror("error forking");
        } else if (child_pid == 0) { // child
            printf("argc: %d\n", argc[i]);
            printf("argv -- ");
            for(int j = 0; j < argc[i]; j++) {
                int index = j+argv_i[i];
                printf("index: %d\n", index);
                printf("%d: %s,", j, argv[index]);
            }
            printf("END\n");
            int dup_err = -1;
            if (i+1 == commands) {
                // dont dup stdout
                printf("last process\n");
                int p_index = (i-1)*2;
                printf("p_index: %d\n", p_index);
                dup_err = dup2(pd[p_index], STDIN_FILENO);
                // close(pd[i*2+1]);
                if(dup_err < 1)
                    perror("dup err 1");
            } else if (i == 0) {
                // dont dup stdin
                printf("first process\n");  
                dup_err = dup2(pd[1], STDOUT_FILENO);
                // close(pd[0]);
                if(dup_err < 1)
                    perror("dup err 2");
            } else {
                // dup both
                printf("process #%d\n", i);
                dup_err = dup2(pd[(i-1)*2], STDIN_FILENO);
                if(dup_err < 1)
                    perror("dup err 3");
                dup_err = dup2(pd[(i)*2+1], STDOUT_FILENO);
                if(dup_err < 1)
                    perror("dup err 4");
            }
            int v_index = argv_i[i];
            printf("v_index: %d\n", v_index);
            int w_index = (i)*2+1;
            int r_index = (i-1)*2;
            printf("w_index: %d\n", w_index);
            printf("r_index: %d\n", r_index);
            for(int i = 0; i < PIPECNTMAX*2; i++) {
                close(pd[i]);
            }
            execvp(argv[v_index], &argv[v_index]);
        } else {
            // test command ls | grep shell | sort -dr
        }
    }
    for(int i = 0; i < PIPECNTMAX*2; i++) {
        close(pd[i]);
    }
    int wait_pid = -1;
    while((wait_pid = wait(NULL)) > 0) printf("waiting on: %d\n", wait_pid);
    printf("after wait\n");
}

int first_ws(char *str)
{
    int len = strlen(str);
    for (int i = 0; i < len; i++)
    {
        if (str[i] == '\n' || str[i] == '\t' || str[i] == ' ')
        {
            return i;
        }
    }
    return len;
}

int main(int argc, char **argv)
{
    char buf[BUFFERSIZE];
    int *myargc = calloc(PIPECNTMAX + 1, sizeof(int));
    char **myargv = calloc(ARGVMAX, sizeof(char *));
    print_prompt();
    while (fgets(buf, BUFFERSIZE, stdin))
    {
        int len = strip_nl(buf, BUFFERSIZE);
        parse(buf, myargc, myargv);
        if (*myargv != NULL && first_ws(*myargv) > 0)
        {
            exec_children(myargv, myargc);
        }
        memset(myargv, 0, ARGVMAX);
        memset(myargc, 0, PIPECNTMAX + 1);
        print_prompt();
    }
    printf("\n");
    free(myargc);
    for (int i = 0; i < ARGVMAX; i++)
    {
        free(myargv[i]);
    }
    free(myargv);
    return 0;
}
