#include "def.h"

struct SMessage msgIn, msgOut;

struct SMyTrakt{
	int myTrakt;	//Nr traktu, którym legion ma przejść
	int priority;	//Priorytet, z jakim proces ubiega się o trakt
};

int sum;			//Suma legionistów obecnych na trakcie
int barier;			//Na ile odpowiedzi czeka proces
int waiting[];		//Procesy, do których proces musi wysłać ‘A’ i/lub ‘L’

void receives() {
	while(true) {
		//receive(-1, &msgIn);
		pvm_recv(-1, MSG_MSTR); //tutaj raczej nie master
		pvm_upkbyte(msgIn.type);
		pvm_upkint(&msgIn.PID, 1, 1);
		pvm_upkint(&msgIn.T, 1, 1);
		pvm_upkint(&msgIn.ID, 1, 1);

		if(trakts[msgIn.T] <= msgIn.ID) trakts[msgIn.T] = msgIn.ID + 1;

		if(msgIn.type == ‘R’) {
			pvm_initsend(PvmDataDefault);
			if(msgIn.T == myTrakt) {
				if(state == ON_TRAKT) {
					pvm_pkbyte('A', 1, 1);
					pvm_pkint(&pID, 1, 1);
					pvm_pkint(&msgIn.T, 1, 1);
					pvm_pkint(&R, 1, 1);
					pvm_send(msgIn.PID, pID); //tu chyba coś inaczej
					/*msgOut.type = ‘A’;
					msgOut.PID = pID;
					msgOut.T = msgIn.T;
					msgOut.ID = R;
					send(msgIn.PID, msgOut);*/
					waiting.push(msgIn.PID); //waiting.append(msgIn.PID);
				}

				if(state == STARTS) {
					if(msgIn.ID < priority || (msgIn.ID == priority && msgIn.PID < pID) ) {
						pvm_pkbyte('A', 1, 1);
						pvm_pkint(&pID, 1, 1);
						pvm_pkint(&msgIn.T, 1, 1);
						pvm_pkint(0, 1, 1);
						pvm_send(msgIn.PID, pID); //tu chyba coś inaczej
						/*msgOut.type = ‘A’;
						msgOut.PID = pID;
						msgOut.T = msgIn.T;
						msgOut.ID = 0;
						send(msgIn.PID, msgOut);*/
					} else {
						waiting.push(msgIn.PID); //waiting.append(msgIn.PID);
					}
				}
			} else {
				pvm_pkbyte('A', 1, 1);
				pvm_pkint(&pID, 1, 1);
				pvm_pkint(&msgIn.T, 1, 1);
				pvm_pkint(0, 1, 1);
				pvm_send(msgIn.PID, pID); //tu chyba coś inaczej
				/*msgOut.type = ‘A’;
				msgOut.PID = pID;
				msgOut.T = msgIn.T;
				msgOut.ID = 0;
				send(msgIn.PID, msgOut);*/
			}
		} else if(msgIn.type == ‘L’) {
			if(msgIn.T == myTrakt) {
				sum -= msgIn.ID;
			}
		} else {
			if(msgIn.T == myTrakt) {
				sum += msgIn.ID;
				barier--;
			}
		}
	}
}

void Enter() {
	state = STARTS;
	priority = trakts[myTrakt].ID;
	trakts[myTrakt].ID++;

	barrier = L - 1;

	pvm_initsend(PvmDataDefault);
	pvm_pkbyte('R', 1, 1);
	pvm_pkint(&pID, 1, 1);
	pvm_pkint(&myTrakt, 1, 1);
	pvm_pkint(&priority, 1, 1);
	/*msgOut.type = ‘R’;
	msgOut.ID = priority;*/

	for(int i = 0; i < L; i++) {
		if(i!=pID) {
			pvm_send(i, pID);
			//send(i, msgOut);
		}
	}

	while(barrier > 0);
	while( sum + R >  trakts[myTrakt].t);
}

void Go() {
	for(int i = 0; i < waiting.size(); i++) {
		pvm_initsend(PvmDataDefault);
		pvm_pkbyte('A', 1, 1);
		pvm_pkint(&pID, 1, 1);
		pvm_pkint(&myTrakt, 1, 1);
		pvm_pkint(&L, 1, 1);
		pvm_send(waiting[i], pID);
		/*msgOut.type = ‘A’;
		msgOut.PID = pID;
		msgOut.T = myTrakt;
		msgOut.ID = L;
		send(waiting[i], msgOut);*/
	}

	state = ON_TRAKT;
	wait(trakts[myTrakt].time);

	for(int i = 0; i < waiting.size(); i++) {
		pvm_initsend(PvmDataDefault);
		pvm_pkbyte('L', 1, 1);
		pvm_pkint(&pID, 1, 1);
		pvm_pkint(&myTrakt, 1, 1);
		pvm_pkint(&L, 1, 1);
		pvm_send(waiting[i], pID);
		/*msgOut.type = ‘L’;
		msgOut.PID = pID;
		msgOut.T = myTrakt;
		msgOut.ID = L;
		send(waiting[i], msgOut);*/
	}
	waiting.clear();
}

void Rest() {
	myTrakt = -1;
	state = WAITS;
	wait(random());
}

main() {

}
