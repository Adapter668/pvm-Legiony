#include "def.h"

struct SMessage msgIn, msgOut;
struct STrakt trakts[T];	//lista traktów
struct SLegion legions[L];	//lista legionów
int tids[L];				//Identyfikatory procesów potomnych
FILE * file;

void intHandler(int sig) {
	printf("Koncze prace...\n");
	for (i = 0; i < L; i++)
	{
		if (tids[i] > 0 && tids[i] < 2 * L)
		{
			pvm_kill(tids[i]);	//Zakonczenie pracy uruchomionych procesow
			printf("Zamykam proces %d o PID %d\n", i,tids[i]);
		}
	}
	pvm_exit();

	if (file != NULL)
		fclose(file);
	exit(1);
}

//Przygotowuje parametry legionistów i traktów
void ConfigInit(){
	if (L <= T)
	{
		printf("Zbyt malo legionow! Legionow: %d, Traktow: %d\n", L, T);
		exit(1);
	}

	srand(time(NULL));

	int i;
	int mint = rand() % 100 + 20;
	int maxl = mint - 1;
	int sumt = T * mint;
	int suml = L * maxl;

	//Legioniści
	for (i = 0; i < L; i++)
		legions[i].R = maxl;

	int todel = rand() % (suml - sumt - 1);
	for (i = 0; i < todel; i++)
	{
		legions[rand() % L].R--;
		suml--;
	}

	//Trakty
	for (i = 0; i < T; i++)
	{
		trakts[i].t = mint;
		trakts[i].time = rand() % 100 + 20;
	}

	todel = rand() % (suml - sumt - 1);
	for (i = 0; i < todel; i++)
	{
		trakts[rand() % T].t++;
		sumt++;
	}

	//Sprawdzenie poprawności
	suml = 0;
	maxl = 0;
	for (i = 0; i < L; i++)
	{
		if (legions[i].R < 0)
		{
			printf("Legion z ujemna liczba legionistow!\n");
			exit(1);
		}

		suml += legions[i].R;
		if (legions[i].R > maxl)
		{
			maxl = legions[i].R;
		}
	}

	sumt = 0;
	mint = 150;
	for (i = 0; i < T; i++)
	{
		if (trakts[i].t < 0)
		{
			printf("Trakt z ujemna przepustowoscia!\n");
			exit(1);
		}

		sumt += trakts[i].t;
		if (trakts[i].t < mint)
		{
			mint = trakts[i].t;
		}
	}

	//min(ti) > max(Ri) 
	if (mint <= maxl)
	{
		printf("Minimalny trakt nie miesci najwiekszego legionu! Legionistow: %d, Traktow: %d\n", maxl, mint);
		exit(1);
	}

	//suma(ti) < suma(Ri).
	if (sumt >= suml)
	{
		printf("Blad sum! Legionistow: %d, Traktow: %d\n", suml, sumt);
		exit(1);
	}
}

main(){
	
	int i;						//Zmienna sterująca pętlami

	ConfigInit();

	//Uruchomienie slaveow
	int mytid = pvm_mytid();	//Rejestracja mastera w MW
	int nproc=pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", L, tids);
	
	if (nproc < L)				//Sprawdzenie poprawnosci uruchomienia procesow slave
	{
		for (i = 0; i < L; i++)
			if (tids[i] > 0 && tids[i] < 2 * L)
			{
				pvm_kill(tids[i]);	//Zakonczenie pracy uruchomionych procesow
				printf("Zamykam proces %d o PID %d\n", i, tids[i]);
			}
		printf("Uruchomienie %d legionow zakonczylo sie bledem. Uruchomiono %d procesow\n", L, nproc);
		exit(1);
	}

	//Wysłanie do procesów informacji o feedbacku
	int feedback = FEEDBACK_ON;
	pvm_initsend(PvmDataDefault);
	pvm_pkint(&feedback, 1, 1);
	pvm_mcast(tids, L, MSG_MSTR);
	
	//Wysłanie do procesów informacji o legionach
	for (i = 0; i < L; i++){
		legions[i].PID = tids[i];
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&legions[i].R, 1, 1);
		pvm_send(legions[i].PID, MSG_MSTR);
	}

	//Wysłanie do procesów informacji o traktach
	for (i = 0; i < T; i++){
		pvm_initsend(PvmDataDefault);
		pvm_pkint(&trakts[i].t, 1, 1);
		pvm_pkint(&trakts[i].time, 1, 1);
		pvm_mcast(tids, L, MSG_MSTR);
	}

	//Odbieranie wiadomosci o wchodzeniu/schodzeniu z trasy + zapis do pliku
	FILE * file = fopen("stats.csv", "w");
	if (file == NULL)
	{
		printf("Nie udalo sie otworzyc pliku stats.csv\n");
		exit(1);
	}


	fprintf(file, "%d;%d\n", T, L);
	for (i = 0; i < T; i++)
		fprintf(file, "T;%d;%d\n", i, trakts[i].t);
	for (i = 0; i < L; i++)
		fprintf(file, "L;%d;%d\n", legions[i].R, legions[i].PID);

	while (1)
	{
		int bufid = pvm_recv(-1, MSG_SLV);
		pvm_upkbyte(&msgIn.type);
		pvm_upkint(&msgIn.PID, 1, 1);
		pvm_upkint(&msgIn.T, 1, 1);
		pvm_upkint(&msgIn.ID, 1, 1);

		fprintf(file, "%c;%d;%d;%d\n", msgIn.type, msgIn.PID, msgIn.T, msgIn.ID);
	}
	fclose(file);
	pvm_exit();					//Opuszczenie maszyny wirtualnej przez mastera
}
