#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 

#define MAX_PIDS 10
#define MAX_ARGS 10

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
    char *line = (char*) malloc(sizeof(char));
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    if (length <= 0) {
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
            args[i++] = token;
    		}
		}
		for (int j=i+1; j<20; j++) args[i]=NULL; //erase the leftover arguments from the last time we called this function
    return i;
}

void freecmd(char** args){
	if (strlen(args[0]) > 0) {
		free(args[0]);
	}
}

struct indexed_string {
	int index; 
	char string[50];
};

void print_indexed_strings(struct indexed_string** entries, int length){
	for (int i=0; i<length; i++){
		printf("String: %s with index %d at array position %d \n", entries[i]->string, entries[i]->index, i);
	}
}

void print_int_array(int* numbers, int length){
	int job=1;
	for (int i=0; i<length; i++){
printf("number %d kill %d\n", numbers[i], kill(numbers[i],0));
		if (numbers[i] && kill(numbers[i], 0)){
			printf("job %d: %d \n", job, numbers[i]);
			job++;
		}
	}
}

void erase_process(int* numbers, int elt, int length){
	for (int i=0; i<length; i++){
		if (numbers[i]==elt) numbers[i]=0;
	}
}

int store_command(struct indexed_string** entries, char* string, int* array_index, int* string_index) {
	//initialize memory for indexed_string entry
printf("store_command is called");
	if(entries[*array_index] == NULL){
printf("DEBUG entries[%d] is null", *array_index);
		entries[*array_index] = (struct indexed_string*)malloc(sizeof(struct indexed_string));
	}
printf("DEBUG history memory allocated");
	if(*string) {
		strcpy(entries[*array_index]->string, string); //save the string (maybe overwrite the previous string stored here)
		entries[*array_index]->index=*string_index; //set the index of the string we're storing
	}
	
	else return 0;

	return 1;
}

/* determines if arguments have a redirect operator
 *
 * return status:
 * 	  0 redirection operator '<' not found
 *	 -1 operator is present but on the boundary; it's not separating two arguments
 * 	 -2 there are two or more reidrection operators present
 *
 */
int output_redirected(char** args){
	int redirect, num = 0;

	/* check for valid syntax: '<' must separate two arguments */
	if ((strcmp(args[0],"<") == 0) || (strcmp(args[MAX_ARGS],"<") == 0)) {
		exit(-1);
	}

	/* split up arguments for redirection and check for valid syntax: there can only be one redirection */
	for (int i=1; i<(MAX_ARGS-1); i++){
		if (strcmp(args[i],"<")) {
			redirect = i;	
			num++;	
		}
	}
	if (num>1) exit(-2);

	return redirect;
}	
	

int main()
{
    char *args[20];

		struct indexed_string* history[10];	
		int hist_entry, hist_index = 0; //history_index counts modulo 10 in the struct history array. hist_entry counts the number of the current command we're storing

		for(int i=0; i<10; i++) history[i]=(struct indexed_string*) malloc(sizeof(struct indexed_string)); //this breaks getline() in getcmd() so that i need to allocate *line before it works

    int bg, cnt, child_return, save_history;
		pid_t child_pid;
		pid_t pids[MAX_PIDS];
		int pid_index=0;
		

		while (1)
		{
			/* the steps are:
			(1) fork a child process using fork()
			(2) the child process will invoke execvp()
			(3) if background == 0, the parent will wait,
			otherwise gets the next command... */
			bg = 0;
			save_history = 1;
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
			else if(!strcmp(*args,"history")) {
				print_indexed_strings(history,10);
			}
			else if(!strcmp(*args,"fg")){
				if ((child_pid = atoi(args[1]))) {
					waitpid(child_pid,&child_return,0);
					erase_process(pids, child_pid, MAX_PIDS); //erase process id from list of background processes
				}
			}
			else if(!strcmp(*args,"jobs")){
				print_int_array(pids, MAX_PIDS);
			}
			else {	
			// Fork and create CHILD PROCESS
				if ((child_pid = fork())) {
					/* parent doesn't wait: save child process to list */
					if (bg) {
						pids[pid_index]=child_pid;
						pid_index = (pid_index + 1) % MAX_PIDS;
					}
					/* parent waits */
					else {	
						waitpid(child_pid, &child_return, 0); //wait for child if int bg is set to 0 
						if (child_return) save_history=0; //child_return is false only if execvp executed an erroneous command
					}
				}
				else {
	printf("CHILD PROCESS\n");
					if ((int n = output_redirected(args))){
						fclose(stdout);
						if (!open(args[n+1])) exit(-1); //unable to redirect output
					}
					execvp(*args, args); 
					exit(-1);
				}
			}	

			// c processes logical operators from left to right 
			if (save_history && store_command(history, *args, &hist_index, &hist_entry)) { 
				printf("history index: %d history string: %s\n",history[hist_index]->index,history[hist_index]->string);
				hist_entry++;
				hist_index=(hist_index+1)%10;
printf("hist index %d\n", hist_index);
			}
			print_indexed_strings(history,10);

			freecmd(args);
	}	
}

