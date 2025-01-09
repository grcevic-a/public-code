#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int Id; // identifikacijski broj segmenta za A
int Id_p;
int Id_z;
int *A; // zajd varijabla
int M; //broj ponavljanja petlje
int *pravo; //varijabla koja odreduje koja dretva, tj. proces moze uci u K.O.
int *zastavica; //zastavice

void proces_prvi(int br) {
	for (int i = 0; i < M; i++) {
		zastavica[br] = 1;
		while (zastavica[1 - br] != 0) {
			if (*pravo != br) {
				zastavica[br] = 0;
				while (*pravo != br) {
					;
				}
				zastavica[br] = 1;
			}
		}
		
		(*A)++;
		int pom = *A;
		printf("Proces %d uvecava zajd. varijablu:  A = %d\n", br, pom);
		*pravo = 1 - br;
		zastavica[br] = 0;
		sleep(1);
	}
}

void obradi_sigint(int sig) {
	printf("Primio signal SIGINT, prekidam rad.\n");
	exit(0);
}

void brisi(int sig) {
   // oslobađanje zajedničke memorije
   (void) shmdt((char *) A);
   (void) shmctl(Id, IPC_RMID, NULL);
   (void) shmdt((char *) pravo);
   (void) shmctl(Id_p, IPC_RMID, NULL);
   (void) shmdt((char *) zastavica);
   (void) shmctl(Id_z, IPC_RMID, NULL);
   
   exit(0);
}

int main(int argc, char** argv) {
	
	struct sigaction act;
	
	M = atoi(argv[1]);
	
	// zauzimanje zajedničke memorije
	Id = shmget(IPC_PRIVATE, sizeof(int), 0600);
	if (Id == -1) {
		exit(1);   // greska
	}
	A = (int *) shmat(Id, NULL, 0);
	
	Id_p = shmget(IPC_PRIVATE, sizeof(int), 0600);
	pravo = (int *) shmat(Id_p, NULL, 0);
	
	Id_z = shmget(IPC_PRIVATE, 2* sizeof(int), 0600);
	zastavica = (int *) shmat(Id_z, NULL, 0);
	
	*A = 0;
	*pravo = 0;
	zastavica[0] = zastavica[1] = 0;
	
	int pid = fork();
	if (pid == 0) {
		proces_prvi(0);
		exit(0);
	} else if (pid == -1) {
		printf("Greska!");
	}
	
	proces_prvi(1);
	
	(void) wait(NULL);
	brisi(0);
	
	
	return 0;
}
