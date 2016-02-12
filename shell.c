#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 

/* Funciton getcmd()
// INPUTS: string prompt, char**, int*
// OUTPUT: int i, the number of nonempty strings found in args delimited by \t\n
// SIDE EFFECTS:
// 	1) prints to stdout
//	2) use int* background to tell main method if '&' is present in args
//	3) store pointers in **args array
// PURPOSE: 
// 	1) display shell prompt to user
// 	2) get user input, allocate memory and store it, and point line to this new block
// 	3) parse input; delimit by '\t\n'. store an array of pointers in *args[] to each delimited argument. convert each argument to a string by adding null terminator inside *line
*/

int getcmd(char *prompt, char **args, int *background)
{
    int length, i = 0;
    char *token, *loc;
    char *line ;//= (char*) malloc(sizeof(char));
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    if (length <= 0) {
printf("DEBUG: no input");
        exit(-1);
    }

    // Check if background is specified.. consider making this check as part of strsep
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else {
        *background = 0;
		}

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++)
            if (token[j] <= 32) // weird ASCII characters that might delimit arguments have values 32 or less
                token[j] = '\0';
        if (strlen(token) > 0) {
printf("DEBUG token: %s \n", token);
            args[i++] = token;
    		}
		}
		for (int j=i+1; j<20; j++) args[i]=NULL; //erase the leftover arguments from the last time we called this function
		
    return i;
}

void freecmd(char** args){
	if (strlen(args[0]) > 0) {
printf("DEBUG about to free *args\n");
		free(args[0]);
	}
}



int main()
{
    char *args[20];
    int bg, cnt, child_return;
		pid_t child_pid;

		while (1)
		{
			/* the steps are:
			(1) fork a child process using fork()
			(2) the child process will invoke execvp()
			(3) if background == 0, the parent will wait,
			otherwise gets the next command... */
			bg = 0;
			cnt = getcmd("\n>>  ", args, &bg);
				
			//Print Args
			for (int i = 0; i < cnt; i++) printf("\nArg[%d] = %s", i, args[i]);
			//Print Background or No Background (& at the end of cmd)
			if (bg) printf("\nBackground enabled..\n");
			else printf("\nBackground not enabled \n");
			printf("\n\n");
		
			//Built-in Commands
			if (!strcmp(*args,"cd")) {
				chdir(args[1]);
			}
			else if(!strcmp(*args,"pwd")) {
				char buffer[100];
				printf("Current working directory is: \n %s \n", getcwd(buffer,sizeof(buffer)));
			}
			else if(!strcmp(*args,"exit")) {
				exit(1);
			}
			else {	
				if ((child_pid = fork())) {
					if (!bg) waitpid(child_pid, &child_return, 0); //wait for child if int bg is set to 0 
				}
				else {
	printf("CHILD PROCESS\n");
					execvp(*args, args); 
					//printf("execvp failed\n"); //printing after a failed execvp causes getcmd() to fail in next command. realloc not allocated error..?
				}
			}	
			freecmd(args);
	}	
}

