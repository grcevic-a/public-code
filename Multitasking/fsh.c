#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void obradi_sigint(int sig);

int pid;

int main() {
	char str[100];
	char *new_string[10];
	char *envp[] = {NULL};
	
	chdir(getenv("HOME"));
	
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = obradi_sigint;
	sigaction(SIGINT, &act, NULL);

	while(1) {
		printf("fsh> ");
		
		fgets(str, 100, stdin);
		str[strcspn(str, "\n")] = 0;  /* https://www.geeksforgeeks.org/removing-trailing-newline-character-from-fgets-input/ */
		
		
		if(strcmp(str, "exit") == 0 || strcmp(str, "") == 0)
			return 0;
		
		char *ptr = strtok(str, " ");
		int i = 0;
		while(ptr != NULL) {	/* https://www.codingame.com/playgrounds/14213/how-to-play-with-strings-in-c/string-split */
			new_string[i] = ptr;
			i++;
			ptr = strtok(NULL, " ");
		}
		new_string[i] = NULL;
			
		if (strcmp(new_string[0], "cd") == 0) {
			if(chdir(new_string[1]) == -1)
				fprintf(stderr, "cd: The directory '%s' does not exist\n", new_string[1]);
		} else {
			pid = fork();
			if (pid == 0) {
				setpgid(0, 0);			
				if (execve(new_string[0], new_string, envp) == -1)
					printf("fsh: Unknown command: %s\n", str);
				exit(0);
			} else if (pid == -1) {
				printf("Greska2!\n");
			}
				
			(void) wait(NULL);
		}
	}
	return 0;

}

void obradi_sigint(int sig){
	if (pid > 0) 
		killpg(pid, SIGINT); 
	printf("\n");
}
