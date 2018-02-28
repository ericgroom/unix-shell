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
}

void parse(char *raw, int *argc, char **argv) {
    int i = 0;
    for(char *token = strtok(raw, " "); token != NULL; token = strtok(NULL, " ")) {
        argv[i] = token;
        i++;
    }
    *argc = i;
}

void start_loop() {
    char buf[BUFFERSIZE];
    int status;
    int* myargc = malloc(sizeof(int));
    char** myargv = calloc(PIPECNTMAX*MAX_ARGV_PER_COMMAND, ARGVMAX);
    char** callingargv = calloc(MAX_ARGV_PER_COMMAND+1, ARGVMAX);
    print_prompt();
    while(fgets(buf, BUFFERSIZE, stdin)) {
        int len = strip(buf, BUFFERSIZE);
        parse(buf, myargc, myargv);
        callingargv = myargv;
        callingargv[4] = NULL;
        pid_t child_pid = -1;
        child_pid = fork();
        if (child_pid == -1) perror("forking error");
        int exec_return = -1;
        execvp(callingargv[0], callingargv);
        if (exec_return < 0) perror("error executing");
        //int pid = wait(&status);
        printf("myargc: %d\n", *myargc);
        for(int i = 0; i < *myargc; i++) {
            printf("contents[%d]: %s\n", i, myargv[i]);
        }
        print_prompt();
    }
    printf("\n");
}

int main(int* argc, char** argv)
{
    start_loop();
    
return 0;
}
