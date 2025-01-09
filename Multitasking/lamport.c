#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

int A; // zajd varijabla
int N, M;
int zadnji_broj;
int *ulaz, *broj;

void *lamport(void* br) {
	int i = *(int*)br;
	ulaz[i] = 1;
	broj[i] = zadnji_broj + 1;
	zadnji_broj = broj[i];
	ulaz[i] = 0;
	
	for (int j = 0; j < M; j++) {
		while (ulaz[j] == 1) {
			;
		}
		while ((broj[j] != 0) && ((broj[j] < broj[i]) || (broj[j] == broj[i] && j < i))) {
			;
		}
		
		A++;
		printf("Dretva br. %d uvecala var A = %d\n", (i+1), A);
		broj[i] = 0;
		sleep(1);
	}
}

int main(int argc, char** argv) {
	struct sigaction act;
	
	N = atoi(argv[1]);
	M = atoi(argv[2]);
	
	ulaz = (int*) malloc(N * sizeof(int));
	broj = (int*) malloc(N * sizeof(int));
	
	pthread_t thr_id[N];
	
	int *id = malloc(N * sizeof(int));
	
	A = 0;
	zadnji_broj = 0;
	
	for (int i = 0; i < N; i++) {
		id[i] = i;
		if (pthread_create(&thr_id[i], NULL, lamport, &id[i])) {
			printf("Greska pri stvaranju dretve!\n");
      			exit(1);
		}
	}
	
	for (int i = 0; i < N; i++) {
		pthread_join(thr_id[i], NULL);
	}

}
