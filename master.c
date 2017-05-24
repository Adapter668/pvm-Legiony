#include "def.h"
struct SMessage msgIn, msgOut;

int pID; 		//Numer PID procesu (nr legionu)
int R;			//liczba legionistów w legionie procesu
int myTrakt;		//Nr traktu, którym legion ma przejść
int state;		//Stan (WAITS, STARTS, ON_TRAKT)
int priority;		//Priorytet, z jakim proces ubiega się o trakt
int sum;		//Suma legionistów obecnych na trakcie
int barier;		//Na ile odpowiedzi czeka proces
vector<int> waiting;

}

main(){
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
