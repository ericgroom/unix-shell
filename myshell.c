/****************************************************************
 * Name        :                                                *
 * Class       : CSC 415                                        *
 * Date  	   :                                                *
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

void print_prompt() {
    printf("%s", PROMPT);
}

void start_loop() {
    char buf[BUFFERSIZE];
    print_prompt();
    while(fgets(buf, BUFFERSIZE, stdin)) {
        printf("contents: %s", buf);
        print_prompt();
    }
    printf("\n");
}

int main(int* argc, char** argv)
{
    start_loop();
    
    
    
    
return 0;
}
