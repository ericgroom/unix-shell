/****************************************************************
 * Name        : Eric Groom                                     *
 * Class       : CSC 415                                        *
 * Date        : 3/12/2018                                      *
 * Description :  Writting a simple bash shell program          *
 *                that will execute simple no_commands. The main*
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
    char *dir = malloc(256);
    memset(dir, 0, 256);
    getcwd(dir, 256);
    char *home = getenv("HOME");
    char *ret = strstr(dir, home);
    if(ret) {
        int index = 0;
        while(index < strlen(home)) {
            if (dir[index] != home[index]) {
                break;
            }
            index++;
        }
        snprintf(dir, 256, "%s%s", "~/", &dir[index+1]);
    }
    printf("%s%s%s", PROMPTNAME, dir, PROMPTSEPARATOR);
    free(dir);
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

int handle_builtins(char **argv) {
    if (strncmp(argv[0], "exit", 4) == 0)
    {
        exit(0);
    }
    else if (strncmp(argv[0], "cd", 2) == 0)
    {
        int chdir_err = -1;
        chdir_err = chdir(argv[1]);
        if (chdir_err < 0)
            perror("error changing directories");
        return 1;
    }
    else if (strncmp(argv[0], "pwd", 3) == 0)
    {
        char buf[256];
        getcwd(buf, 256);
        printf("%s\n", buf);
        return 1;
    }
    return 0;
}

int redirect(char **argv, int* argc) {
    int i = 0;
    while(i < *argc) {
        if (strcmp(argv[i], ">") == 0)
        {
            int filedes = open(argv[i + 1], O_WRONLY | O_CREAT, 0640);
            argv[i] = NULL;
            dup2(filedes, STDOUT_FILENO);
            *argc = i;
            return 1;
        }
        else if (strcmp(argv[i], ">>") == 0)
        {
            int filedes = open(argv[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0640);
            argv[i] = NULL;
            dup2(filedes, STDOUT_FILENO);
            *argc = i;
            return 1;
        }
        else if (strcmp(argv[i], "<") == 0)
        {
            int filedes = open(argv[i + 1], O_RDONLY);
            argv[i] = NULL;
            dup2(filedes, STDIN_FILENO);
            *argc = i;
            return 1;
        }
        i++;
    }
    return -1;
}

void exec_children(char **argv, int *argc)
{
    // no of no_commands
    int no_commands = 0;
    while(no_commands < PIPECNTMAX+1 && argc[no_commands] > 0) no_commands++;

    // create pipes
    int pd[PIPECNTMAX*2];
    for(int i = 0; i < PIPECNTMAX; i++) {
        pipe(&pd[i*2]);
    }

    int wait_index = -1;
    for (int i = 0; i < ARGVMAX; i++)
    {
        if (argv[i] != NULL && strcmp(argv[i], "&") == 0)
        {
            argv[i] = NULL;
            wait_index = i;
            argc[0]--;
            break;
        }
    }
    
    // determine start index for each command
    int argv_i[PIPECNTMAX+1]; // used to determine the start index for each command
    argv_i[0] = 0;
    for(int i = 1; i < no_commands; i++) {
        argv_i[i] = argv_i[i-1] + argc[i-1] + 1;
    }
    // fork loop
    for(int i = 0; i < no_commands; i++) {
        pid_t child_pid = -1;
        int argv_index = argv_i[i];
        if (handle_builtins(&argv[argv_index])) {
            continue;
        }
        child_pid = fork();
        if (child_pid < 0) {
            perror("error forking");
        } else if (child_pid == 0) { // child
            int has_redirect = redirect(&argv[argv_index], &argc[i]);
            if (has_redirect <= 0) {
                if (i+1 == no_commands) {
                    // dont dup stdout
                    int dup_err = -1;
                    if (no_commands > 1) {
                        int p_index = (i-1)*2;
                        dup_err = dup2(pd[p_index], STDIN_FILENO);
                        if(dup_err < 0)
                            perror("dup err 1");
                    }
                } else if (i == 0) {
                    // dont dup stdin
                    int dup_err = -1;
                    dup_err = dup2(pd[1], STDOUT_FILENO);
                    if(dup_err < 0)
                        perror("dup err 2");
                } else {
                    // dup both
                    int dup_err = -1;
                    dup_err = dup2(pd[(i-1)*2], STDIN_FILENO);
                    if(dup_err < 0)
                        perror("dup err 3");
                    dup_err = dup2(pd[(i)*2+1], STDOUT_FILENO);
                    if(dup_err < 0)
                        perror("dup err 4");
                }  
            }          
            for(int i = 0; i < PIPECNTMAX*2; i++) {
                close(pd[i]);
            }

            execvp(argv[argv_index], &argv[argv_index]);
        }
    }
    for(int i = 0; i < PIPECNTMAX*2; i++) {
        close(pd[i]);
    }
    if (wait_index < 0) {
        int wait_pid = -1;
        while((wait_pid = wait(NULL)) > 0);
    }
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
