#include "def.h"

struct SMessage msgIn, msgOut;
struct STrakt trakts[T];	//lista traktów
struct SLegion legions[L];	//lista legionów

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
		trakts[i].t = mint;

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
	ConfigInit();




	//tu trzeba poprzesyłać coś i takoś zainicjować odpalenie legionów (slaveów)
	int tids[L];
	int mytid;
	int i;
	mytid = pvm_mytid();
	nproc=pvm_spawn(SLAVENAME, NULL, PvmTaskDefault, "", L, tids);
	for(i = 0; i < nproc; i++){
	pvm_initsend(PvmDataDefault);
		pvm_pkint(&mytid, 1, 1);
		pvm_send(tids[i], MSG_MSTR);
	}
	pvm_exit();
}
