#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<stdbool.h>

typedef struct{
        	char zapis[64];  //char ima 8 bita
        	int slobodan;
}okvir_struktura;

okvir_struktura **disk;  //disk[N][16]
okvir_struktura *okvir;  //okvir[M]
short **tablica;	 //tablica[N][16]

int N, M, t;


int dohvati_fizicku_adresu(int p, int x, bool upisivanje) {
	int indeks_tablice_stranicenja = (((1 << 4) - 1) & (x >> (7 - 1))); //indeks od 6. do 9. bita
	int pomak = (((1 << 6) - 1) & (x >> (1 - 1))); //pomak od 0. do 5.
	
	int fiz_adresa = 0;
	int zapis_tablice = 0;
	int sadrzaj_adrese = 0;
	bool naden_okvir = false; 
	
	//ako je t 31 mijenjamo sve lru-ove i globalnu varijablu t postavljamo na 0
	if(t == 31) {
		t = 0;
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < 16; j++) {
				if((tablica[i][j] & 0x020) != 0) {
					tablica[i][j] = tablica[i][j] & 0xffe0;
				}
			}
		}
		t++;
	}
	//nadi zapis tablice stranicenja procesa p za adresu x
	//gleda je li bit prisutnosti 0
	if((tablica[p][indeks_tablice_stranicenja] & 0x020) == 0) {
		printf("        Promasaj!\n");
		
		for (int i = 0; i < M; i++) {
			//pronadi i dodijeli okvir
			if (okvir[i].slobodan == 0) {
				okvir[i].slobodan = 1;
				naden_okvir = true;
				printf("                dodijeljen okvir 0x%04x\n", i);
				
				//ucitaj sadrzaj stranice s diska
				for (int j = 0; j < 64; j++) {	
					okvir[i].zapis[j] = disk[p][indeks_tablice_stranicenja].zapis[j];
				}
				
				//azuriraj tablicu prevodenja procesa p;
				tablica[p][indeks_tablice_stranicenja] = (i << 6) | (1 << 5) | t;  //postavljamo vrijednost i od 6. do 15. bita, postavljamo bit prisutnosti u 1 i azuriramo lru
				//printf("0x%04x\n", tablica[p][indeks_tablice_stranicenja]);
				fiz_adresa = (i << 6) | pomak;  //u fiz. adresu postavljamo vrijednost i od 6. bita, tj. vrijednost koja se u tablici prevodenja nalazi do bita prisutnosti + pomak koji iscitamo iz nasumicno generirane adrese
				zapis_tablice = tablica[p][indeks_tablice_stranicenja];
				
				sadrzaj_adrese = okvir[(((1 << 4) - 1) & (zapis_tablice >> (7 - 1)))].zapis[pomak]; //okvir[fizicka adresa okvira].zapis[pomak]
				//printf("0x%04x\n", (i << 6));
				//printf("0x%04x\n", pomak);
				break;
			}
		}
		
		//ako nije pronaden prazni okvir, trazi se onaj koji ima najmanji lru i on se izbacuje
		if(naden_okvir == false) {
			int minLRU = t;
			int pomLRU, proces, zapis, okvir_min;
		
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < 16; j++) {
					pomLRU = (((1 << 5) - 1) & (tablica[i][j] >> (1 - 1)));
					if (pomLRU < minLRU && (tablica[i][j] & 0x020) != 0) {
						minLRU = pomLRU;
						proces = i;
						zapis = j;
						okvir_min = (((1 << 4) - 1) & (tablica[i][j] >> (7 - 1)));
					}
				}
			}
			
			printf("                Izbacujem stranicu 0x%04x iz procesa %d\n", x - pomak, proces);
			printf("                lru izbacene stranice: 0x%04x\n", minLRU);
			printf("                dodijeljen okvir 0x%04x\n", okvir_min);
			
			tablica[proces][zapis] = 0;
			for (int i = 0; i < 64; i++) {	
				//prebacujem sadrzaj iz okvira na disk i praznim okvir
				disk[proces][zapis].zapis[i] = okvir[okvir_min].zapis[i];
				okvir[okvir_min].zapis[i] = 0;
			}
			for (int i = 0; i < 64; i++) {	
				//punim okvir podacima s diska
				okvir[okvir_min].zapis[i] = disk[p][indeks_tablice_stranicenja].zapis[i];
			}
			
			tablica[p][indeks_tablice_stranicenja] = (okvir_min << 6) | (1 << 5) | t;
			fiz_adresa = (okvir_min << 6) | pomak;
			zapis_tablice = tablica[p][indeks_tablice_stranicenja];
			sadrzaj_adrese = okvir[(((1 << 4) - 1) & (zapis_tablice >> (7 - 1)))].zapis[pomak];
			
		}
		printf("        fiz. adresa: 0x%04x\n", fiz_adresa);
		printf("        zapis tabice: 0x%04x\n", zapis_tablice);
		printf("        sadrzaj_adrese: %d\n", sadrzaj_adrese);	
	} else if (upisivanje == true) {
		//ovo sluzi kada dohvacamo fizicku adresu iz funkcije zapisi_vrijednost
		int do_bita = (((1 << 10) - 1) & (tablica[p][indeks_tablice_stranicenja] >> (7 - 1)));
		fiz_adresa = do_bita << 6| pomak;
		
	} else {
		//ovo ako je bit prisutnosti 1 i ne poziva funkcija zapisi_vrijednost vec dohvati_sadrzaj
		tablica[p][indeks_tablice_stranicenja] = tablica[p][indeks_tablice_stranicenja] & 0xffe0;  
		tablica[p][indeks_tablice_stranicenja] = tablica[p][indeks_tablice_stranicenja] | (1 << 5) | t;
		fiz_adresa = ((((1 << 10) - 1) & (tablica[p][indeks_tablice_stranicenja] >> (7 - 1)))) << 6 | pomak;
		zapis_tablice = tablica[p][indeks_tablice_stranicenja];		
		sadrzaj_adrese = okvir[(((1 << 4) - 1) & (zapis_tablice >> (7 - 1)))].zapis[pomak];
	
		printf("        fiz. adresa: 0x%04x\n", fiz_adresa);
		printf("        zapis tabice: 0x%04x\n", zapis_tablice);
		printf("        sadrzaj_adrese: %d\n", sadrzaj_adrese);
	}
	
	return fiz_adresa;
}

int dohvati_sadrzaj(int p, int x) {
	int y = dohvati_fizicku_adresu(p, x, false);
	int i = okvir[(((1 << 4) - 1) & (y >> (7 - 1)))].zapis[(((1 << 6) - 1) & (y >> (1 - 1)))];
	return i;
}

void zapisi_vrijednost(int p, int x, int i) {	
	int y = dohvati_fizicku_adresu(p, x, true);
	okvir[(((1 << 4) - 1) & (y >> (7 - 1)))].zapis[(((1 << 6) - 1) & (y >> (1 - 1)))] = i;
}


int main(int argc, char** argv) {
	srand(time(NULL));

	N = atoi(argv[1]); //broj procesa
	M = atoi(argv[2]); //broj okvira
	
	
	//zauzimanje memorije za disk, okvir i tablicu
	disk = (okvir_struktura **)malloc(N * sizeof(okvir_struktura *));
	okvir = malloc(M * sizeof(okvir_struktura));
	tablica = (short **)malloc(N * sizeof(short *));
	for (int i = 0; i < N; i++) {
    		disk[i] = (okvir_struktura *)malloc(16 * sizeof(okvir_struktura));
    		tablica[i] = (short *)malloc(16 * sizeof(short));
  	}    	
    	
    	
    	for (int i = 0; i < M; i++) {
    		okvir[i].slobodan = 0;
    		for (int j = 0; j < 64; j++) {
    			//inicijalizacija okvira
    			okvir[i].zapis[j] = 0;
    		}
    	}
    	for (int i = 0; i < N; i++) {
    		for (int j = 0; j < 16; j++) {
    			//inicijaliziraj tablicu stranicenja procesa i
    			tablica[i][j] = 0x0000;
    			
    			disk[i][j].slobodan = 0;
    			for (int k = 0; k < 64; k++) {
    				//inicijalizacija diska
    				disk[i][j].zapis[k] = 0;
    			}
    		}
    	}
    	
    	
    	t = 0;
    	
    	while(1) {
    		for (int p = 0; p < N; p++) {
    			printf("---------------------------\n");
    			printf("proces: %d\n", p);
    			printf("        t: %d\n", t);
    			
    			//int x = (rand() % 1024) & 0x3FE;  //nasumicna parna logicka adresa
    			int x = 0x01fe;
    			printf("        log. adresa: 0x%04x\n", x);
    			
    			int i = dohvati_sadrzaj(p, x);
    			i++;
    			zapisi_vrijednost(p, x, i);
    			t++;
    			sleep(1);
    		}
    	}
    	free(okvir);
    	free(disk);
    	free(tablica);
    	return 0;
    	

}
