#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

sem_t *cekaonica;
sem_t *frizerka;
sem_t *iskljucenje;

int ID, N;
int *br, *broj_mjesta, *otvoreno;
int v = 15;

time_t start;

void proces_sat() {
	time_t vrijeme = time(NULL);
	while (vrijeme - start < v) {
		vrijeme = time(NULL);
	}
	sem_post(frizerka);

}

void proces_frizerka() {
	time_t trenutacno;
	while(1) {
		trenutacno = time(NULL);
		if (trenutacno - start >= v && *broj_mjesta == 0) {
			*otvoreno = 0;
			printf("Frizerka: ZATVARAM\n");
			break;
		} else if (*broj_mjesta != 0) {
			sem_wait(cekaonica);
			sem_wait(iskljucenje);
        		*broj_mjesta -= 1;
        		sem_post(iskljucenje);
            		printf("Frizerka: radim na klijentu\n");
            		sleep(3);
            		printf("Frizerka: zavrsavam rad na klijentu\n");
		} else if (trenutacno - start < v && *br == 1) {
			*br=0;
			printf("Frizreka: spavam\n");
			sem_wait(frizerka);
		}
	}
	
}

void proces_klijent(int i) {
	printf("	Klijent(%d): zelim na frizuru\n", i);
	if (*broj_mjesta < 3 && *otvoreno == 1) {
		sem_wait(iskljucenje);
        	*broj_mjesta += 1;
        	sem_post(iskljucenje);
        	sem_post(cekaonica);
        	printf("	Klijent(%d): usao sam u cekaonicu (%d)\n", i, *broj_mjesta);
        	if (*br == 0) {
        		*br=1;
        		sem_post(frizerka);
        	}
	} else {
		printf("	Klijent(%d): danas nista od frizure\n", i);
	}
}

void obrisi(int x) {
    sem_destroy(cekaonica);
    sem_destroy(iskljucenje);
    sem_destroy(frizerka);
    
    shmdt(cekaonica);
    shmdt(iskljucenje);
    shmdt(frizerka);
    shmdt(broj_mjesta);
    shmdt(br);
    
    exit(0);
}

int main(int argc, char** argv) {
	start = time(NULL);
	srand(time(NULL));

	struct sigaction act;
    	act.sa_handler = obrisi;
    	sigaction(SIGINT, &act, NULL);

    	ID = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
    	cekaonica = shmat (ID, NULL, 0);
    	shmctl (ID, IPC_RMID, NULL);
    
    	ID = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
    	iskljucenje = shmat (ID, NULL, 0);
    	shmctl (ID, IPC_RMID, NULL);
    
    	ID = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
    	frizerka = shmat (ID, NULL, 0);
    	shmctl (ID, IPC_RMID, NULL);
    	
    	ID = shmget (IPC_PRIVATE, sizeof(int), 0600);
    	br = (int *) shmat (ID, NULL, 0);
    	shmctl (ID, IPC_RMID, NULL);
    	
    	ID = shmget (IPC_PRIVATE, sizeof(int), 0600);
    	broj_mjesta = (int *) shmat (ID, NULL, 0);
    	shmctl (ID, IPC_RMID, NULL);
    	
    	ID = shmget (IPC_PRIVATE, sizeof(int), 0600);
    	otvoreno = (int *) shmat (ID, NULL, 0);
    	shmctl (ID, IPC_RMID, NULL);
    	
    	sem_init(cekaonica, 1, 0);
    	sem_init(iskljucenje, 1, 1);
    	sem_init(frizerka, 1, 0);

	N = atoi(argv[1]);
	*broj_mjesta = 0;
	*otvoreno = 1;
	*br = 1;
	
	
	printf("Frizerka: otvaram salon\n");
    	printf("Frizerka: postavljam znak OTVORENO\n");
    	int pid_f = fork();
    	if (pid_f == 0) {
        	proces_frizerka();
        	exit(0);
    	} else if(pid_f == -1) {
     		printf("Greska!");
    	} else {
        	for(int i = 1; i <= N; i++) {
        		sleep(rand() % 4);
            		int pid_k = fork();
            		if (pid_k == 0) {
            	    		proces_klijent(i);
            	    		exit(0);
            		} else if(pid_f == -1) {
     	        		printf("Greska!");
     	    		}	
        	}
        	int pid_s = fork();
		if (pid_s == 0) {
			proces_sat();
			exit(0);
		} else if(pid_s == -1) {
     			printf("Greska!");
    		}
    	}
    	for(int i = 1; i <= N+2; i++) {
            	(void) wait(NULL);
        }

    	
    	obrisi(0);
    
    	return 0;

}
