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
#include <string.h>
#include <stdbool.h>

pthread_mutex_t monitor;
pthread_cond_t uvjet[3];
//0-lijevo; 1-desno; 2-brod

int brod[2] = {0, 0};
int strana_obale = 1;

pthread_t dretva_M[50];
pthread_t dretva_K[50];

int l = 0;
int d = 0;
int LO[100] = { };
int DO[100] = { };
int C[7] = { };
int c = 0;

void ispis_obala(int strana) {
	if (strana_obale == 1)
		printf("lijevu obalu: ");
	else
		printf("desnu obalu: ");
	for(int i = 0; i < c; i++) {
		if (C[i] % 10 == 1) {
			printf("M%d ", C[i] / 10);
		} else if (C[i] % 10 == 2) {
			printf("K%d ", C[i] / 10);
		}
	}
	printf("\n");
}

void ispis(int strana) {
	printf("C[");
	if (strana == 0) {
		printf("L");
	} else {
		printf("D");
	}
	printf("] = { ");
	for(int i = 0; i < c; i++) {
		if (C[i] % 10 == 1) {
			printf("M%d ", C[i] / 10);
		} else if (C[i] % 10 == 2) {
			printf("K%d ", C[i] / 10);
		}
	}
	printf("}   ");

	printf("LO = { ");
	for(int i = 0; i < l; i++) {
		if (LO[i] % 10 == 1) {
			printf("M%d ", LO[i] / 10);
		} else if (LO[i] % 10 == 2) {
			printf("K%d ", LO[i] / 10);
		}
	}
	printf("}   ");

	printf("DO = { ");
	for(int i = 0; i < d; i++) {
		if (DO[i] % 10 == 1) {
			printf("M%d ", DO[i] / 10);
		} else if (DO[i] % 10 == 2) {
			printf("K%d ", DO[i] / 10);
		}
	}
	printf("}\n\n");
}

void *misionari_f(void *pom) {
	int *pom2 = pom;
    	int x = *pom2;
    	
	pthread_mutex_lock(&monitor);
	int strana = rand() % 2;

	printf("M%d: dosao na ", x);
	if (strana == 0) {
		printf("lijevu obalu\n");
		LO[l] = (x*10) + 1;
		l++;	
	} else {
		printf("desnu obalu\n");
		DO[d] = (x*10) + 1;
		d++;
	}
	ispis(strana_obale);
	
	while ((brod[0] + 1) < brod[1] || brod[0] + brod[1] == 7 || strana != strana_obale) {
		pthread_cond_wait(&uvjet[strana], &monitor);
	}
	
	printf("M%d: usao u camac\n", x);
	C[c] = (x*10) + 1;
	c++;
	if (strana == 0) {
		for (int i = 0; i < l; i++) {
			if (LO[i] == (x*10) + 1) {
				LO[i] = 0;
			}
		}
	} else {
		for (int i = 0; i < d; i++) {
			if (DO[i] == (x*10) + 1) {
				DO[i] = 0;
			}
		}
	}
	ispis(strana_obale);
	
	brod[0]++;
	if(brod[0] + brod[1] >= 3) {
		pthread_cond_broadcast(&uvjet[2]);
	}
	pthread_mutex_unlock(&monitor);
}

void *kanibali_f(void *pom) {
	int *pom2 = pom;
    	int x = *pom2;
    	
	pthread_mutex_lock(&monitor);
	int strana = rand() % 2;

	printf("K%d: dosao na ", x);
	if (strana == 0) {
		printf("lijevu obalu\n");
		LO[l] = (x*10) + 2;
		l++;
	} else {
		printf("desnu obalu\n");
		DO[d] = (x*10) + 2;
		d++;
	}
	ispis(strana_obale);
	
	while ((brod[0] <= brod[1] && brod[0] !=0) || brod[0] + brod[1] == 7 || strana != strana_obale) {
		pthread_cond_wait(&uvjet[strana], &monitor);
	}
	
	printf("K%d: usao u camac\n", x);
	C[c] = (x*10) + 2;
	c++;
	if (strana == 0) {
		for (int i = 0; i < l; i++) {
			if (LO[i] == (x*10) + 2) {
				LO[i] = 0;
			}
		}
	} else {
		for (int i = 0; i < d; i++) {
			if (DO[i] == (x*10) + 2) {
				DO[i] = 0;
			}
		}
	}
	ispis(strana_obale);
	brod[1]++;
	if(brod[0] + brod[1] >= 3) {
		pthread_cond_broadcast(&uvjet[2]);
	}
	
	pthread_mutex_unlock(&monitor);
}

void *brod_f(void* arg) {
	while(1) {
		pthread_mutex_lock(&monitor);
		while(brod[0] + brod[1] < 3) {
			pthread_cond_wait(&uvjet[2], &monitor);
		}
		printf("C: tri putnika ukrcana, polazim za jednu sekundu\n");
		ispis(strana_obale);
		sleep(1);
		printf("C: prevozim na ");
		ispis_obala(strana_obale);
		printf("\n");
		sleep(1);
		
		pthread_cond_broadcast(&uvjet[1-strana_obale]);
		brod[0] = 0;
		brod[1] = 0;
		printf("C: prevezao na ");
		ispis_obala(strana_obale);
		if (strana_obale == 1) {
			strana_obale = 0;
		} else {
			strana_obale = 1;
		}
		
		
		printf("C: prazan na ");
		if (strana_obale == 0)
			printf("lijevoj obali\n");
		else
			printf("desnoj obali\n");		
		for (int i = 0; i < c; i++) {
			C[i] = 0;
		}
		c = 0;
		ispis(strana_obale);
		
		pthread_mutex_unlock(&monitor);
	}
}

void *m(void* arg) {
	for (int i = 0; i < 50; i++) {
		sleep(2);
		pthread_create(&dretva_M[i], NULL, &misionari_f, &i);
	}
	for (int i = 0; i < 50; i++) {
		pthread_join(dretva_M[i], NULL);
	}
}

void *k(void* arg) {
	for (int i = 0; i < 50; i++) {
		sleep(1);
		pthread_create(&dretva_K[i], NULL, &kanibali_f, &i);
	}
	for (int i = 0; i < 50; i++) {
		pthread_join(dretva_K[i], NULL);
	}
}


int main(void) {
	srand(time(NULL));
	
	pthread_t dretva_id[3];
	pthread_mutex_init (&monitor, NULL);
	pthread_cond_init (&uvjet[0], NULL);
	pthread_cond_init (&uvjet[1], NULL);
	pthread_cond_init (&uvjet[2], NULL);
	
	printf("C: prazan na desnoj obali\n");
	printf("C[D] = { }   LO={ }   DO={ }\n\n");
	pthread_create(&dretva_id[0], NULL, &brod_f, NULL);
	pthread_create(&dretva_id[1], NULL, &m, NULL);
	pthread_create(&dretva_id[2], NULL, &k, NULL);

	for(int i = 0; i < 3; i++) {
        	pthread_join(dretva_id[i], NULL);
    	}
	
	return 0;
}
