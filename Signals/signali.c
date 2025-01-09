#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

struct timespec t0; /* vrijeme pocetka programa */

/* funkcije za obradu signala, navedene ispod main-a */
void obradi_dogadjaj(int sig);
void obradi_sigterm(int sig);
void obradi_sigint(int sig);

int nije_kraj = 1;
char kz[] = "000";
int tp = 0;
int stog[20] = { };
int n = 0;
bool manje = false;

/* postavlja trenutno vrijeme u t0 */
void postavi_pocetno_vrijeme()
{
	clock_gettime(CLOCK_REALTIME, &t0);
}

/* dohvaca vrijeme proteklo od pokretanja programa */
void vrijeme(void)
{
	struct timespec t;

	clock_gettime(CLOCK_REALTIME, &t);

	t.tv_sec -= t0.tv_sec;
	t.tv_nsec -= t0.tv_nsec;
	if (t.tv_nsec < 0) {
		t.tv_nsec += 1000000000;
		t.tv_sec--;
	}

	printf("%03ld.%03ld:\t", t.tv_sec, t.tv_nsec/1000000);
}

/* ispis kao i printf uz dodatak trenutnog vremena na pocetku */
#define PRINTF(format, ...)       \
do {                              \
  vrijeme();                      \
  printf(format, ##__VA_ARGS__);  \
}                                 \
while(0)

void isprintaj() {
	PRINTF("K_Z=%s, T_P=%d, stog: ", kz, tp);
	if (stog[0] == 0 && n == 0) {
		printf("-");
	} else {
		for (int i = n-1; i >= 0; i--) {
			printf("%d, reg[%d]; ", stog[i], stog[i]);
		}
	}
	printf("\n\n");
}

void spavaj(time_t sekundi)
{
	struct timespec koliko;
	koliko.tv_sec = sekundi;
	koliko.tv_nsec = 0;

	while (nanosleep(&koliko, &koliko) == -1 && errno == EINTR) {
		if (manje == false) {
			PRINTF("Nastavlja se obrada prekida razine %d\n", tp);
			isprintaj();
		} else {
			manje = false;
		}
	}
}

void stavi(int prioritet) {
	if (prioritet == 0 && n == 0) {
		n++;
	} else {
		stog[n] = prioritet;
		n++;
	}
}

void skini() {
	if (n > 0) {
		tp = stog[n-1];
		stog[n-1] = 0;
		n--;
	}
}

int main()
{
	struct sigaction act;

	/* 1. maskiranje signala SIGUSR1 */

	/* kojom se funkcijom signal obradjuje */
	act.sa_handler = obradi_dogadjaj;

	/* koje jos signale treba blokirati dok se signal obradjuje */
	sigemptyset(&act.sa_mask);
	// sigaddset(&act.sa_mask, SIGTERM);

	act.sa_flags = 0; /* naprednije mogucnosti preskocene */

	/* maskiranje signala - povezivanje sucelja OS-a */
	sigaction(SIGUSR1, &act, NULL);

	/* 2. maskiranje signala SIGTERM */
	act.sa_handler = obradi_sigterm;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);

	/* 3. maskiranje signala SIGINT */
	act.sa_handler = obradi_sigint;
	sigemptyset(&act.sa_mask);
	sigaction(SIGINT, &act, NULL);
	
	postavi_pocetno_vrijeme();

	PRINTF("Program s PID=%ld krenuo s radom\n", (long) getpid());
	isprintaj();
	
	int val;
	while(1) {
		if (strcmp(kz, "000") != 0){
			do {
				val = strcmp(kz, "000");
				PRINTF("Nastavlja se izvodenje glavnog programa\n");
				isprintaj();
				if (kz[0] == '1' || kz[1] == '1') {
					if (kz[0] == '1') {
						kz[0] = '0';
						stavi(tp);
						tp = 1;
						obradi_sigterm(1);
					} else if (kz[1] == '1') {
						kz[1] = '0';
						stavi(tp);
						tp = 2;
						obradi_dogadjaj(1);
					} 
				}
			} while (val != 0);
		}
	}
	
	return 0;
}

void obradi_dogadjaj(int sig) //razina 2
{
	if (tp < 2){
		PRINTF("SKLOP: Dogodio se prekid razine 2 i prosljeduje se procesoru\n");
		kz[1] = '1';
		isprintaj();
		
		PRINTF("Pocela obrada prekida razine 2\n");
		kz[1] = '0';
		stavi(tp);
		tp = 2;
		isprintaj();
		
		spavaj(10);
		PRINTF("Zavrsila obrada razine 2\n");
		skini();
	} else if (tp == 2) {
		PRINTF("SKLOP: Promijenio se T_P, prosljeduje prekid razine 2 procesoru\n");
		PRINTF("Pocela obrada prekida razine 2\n");
		isprintaj();
		
		spavaj(5);
		PRINTF("Zavrsila obrada razine 2\n");
		skini();
	} else {
		PRINTF("SKLOP: Dogodio se prekid razine 2, ali on se pamti i ne prosljeduje procesoru\n");
		manje = true;
		kz[1] = '1';
		isprintaj();
	}
		
}

void obradi_sigterm(int sig) //razina 1
{
	if (tp < 1){
		PRINTF("SKLOP: Dogodio se prekid razine 1 i prosljeduje se procesoru\n");
		kz[0] = '1';
		isprintaj();
		
		PRINTF("Pocela obrada prekida razine 1\n");
		kz[0] = '0';
		stavi(tp);
		tp = 1;
		isprintaj();
		
		spavaj(10);
		PRINTF("Zavrsila obrada razine 1\n");
		skini();
	} else if (tp == 1) {
		PRINTF("SKLOP: Promijenio se T_P, prosljeduje prekid razine 1 procesoru\n");
		PRINTF("Pocela obrada prekida razine 1\n");
		isprintaj();
		
		spavaj(5);
		PRINTF("Zavrsila obrada razine 1\n");
		skini();
	} else {
		PRINTF("SKLOP: Dogodio se prekid razine 1, ali on se pamti i ne prosljeduje procesoru\n");
		manje = true;
		kz[0] = '1';
		isprintaj();
	}
}

void obradi_sigint(int sig) //razina 3
{

	PRINTF("SKLOP: Dogodio se prekid razine 3 i prosljeduje se procesoru\n");
	printf("%d", sig);
	kz[2] = '1';
	isprintaj();
	
	PRINTF("Pocela obrada prekida razine 3\n");
	kz[2] = '0';
	stavi(tp);
	tp = 3;
	isprintaj();
	
	spavaj(10);
	PRINTF("Zavrsila obrada razine 3\n");
	skini();
}

