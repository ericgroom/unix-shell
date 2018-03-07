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

void print_prompt() {
    printf("%s", PROMPT);
}

int strip(char *str, int size) {
    for(int i = 0; i < size; i++) {
        if (str[i] == '\n') {
            str[i] = 0;
            return i;
        }
    }
    return size+1;
}

void parse(char *raw, int *argc, char **argv) {
    int command = 0;
    char *saveptr1, *saveptr2;
    for(char *token = strtok_r(raw, "|", &saveptr1); token != NULL; token = strtok_r(NULL, "|", &saveptr1)) {
        for(char *subtoken = strtok_r(token, " ", &saveptr2); subtoken != NULL; subtoken = strtok_r(NULL, " ", &saveptr2)) {
            int index = argc[command] + command*MAX_ARGV_PER_COMMAND;
            argv[index] = subtoken;
            argc[command]++;
        }
        command++;
    }
}

void exec_child(char** argv, int argc) {
    pid_t child_pid = -1;
        child_pid = fork();
        if (child_pid < 0) {
            perror("forking error");
        } else if (child_pid == 0) {
        int exec_return = -1;
            exec_return = execvp(argv[0], argv);
        if (exec_return < 0) perror("error executing");
        } else {
            wait(NULL); // wait for child
        }
}

void start_loop() {
    char buf[BUFFERSIZE];
    int status;
    int* myargc = calloc(PIPECNTMAX, sizeof(int));
    char** myargv = calloc(PIPECNTMAX*MAX_ARGV_PER_COMMAND, ARGVMAX);
    char** callingargv = calloc(MAX_ARGV_PER_COMMAND+1, ARGVMAX);
    print_prompt();
    while(fgets(buf, BUFFERSIZE, stdin)) {
        int len = strip(buf, BUFFERSIZE);
        parse(buf, myargc, myargv);
        printf("myargc: %d\n", *myargc);
        for(int i = 0; i < 16; i++) {
            printf("contents[%d]: %s\n", i, myargv[i]);
        }
        memcpy(callingargv, myargv, myargc[0]);
        callingargv[4] = NULL;
        exec_child(callingargv, myargc[0]);
        
        printf("myargc: %d\n", *myargc);
        for(int i = 0; i < 16; i++) {
            printf("contents[%d]: %s\n", i, myargv[i]);
        }
        memset(myargv, 0, ARGVMAX);
        memset(myargc, 0, ARGVMAX);
        print_prompt();
    }
    printf("\n");
}

int main(int argc, char** argv) {
    start_loop();
    return 0;
}
