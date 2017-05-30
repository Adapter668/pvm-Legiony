#include "def.h"

struct SMessage msgIn, msgOut;

struct SMyTrakt{
	int n;	//Nr traktu, którym legion ma przejść
	int priority;	//Priorytet, z jakim proces ubiega się o trakt
}myTrakt;

int sum;			//Suma legionistów obecnych na trakcie
int barier;			//Na ile odpowiedzi czeka proces
int waiting[];		//Procesy, do których proces musi wysłać ‘A’ i/lub ‘L’

struct SLegion legion;
struct STrakt trakts[T];

//Przygtowywanie wiadomości do wysłania
void PrepareMessage()
{
	pvm_initsend(PvmDataDefault);
	pvm_pkbyte(&msgOut.type, 1, 1);
	pvm_pkint(&msgOut.pID, 1, 1);
	pvm_pkint(&msgOut.t, 1, 1);
	pvm_pkint(&msgOut.iD, 1, 1);
}

/*Odpakowanie wiadomości 
Wiadomośc musi być już w buforze odbiorczym!- polecenie pvm_recv lub pvm_trecv przed wywołaniem procedury*/
void UnpackMessage()
{
	pvm_upkbyte(&msgIn.type);
	pvm_upkint(&msgIn.pID, 1, 1);
	pvm_upkint(&msgIn.t, 1, 1);
	pvm_upkint(&msgIn.iD, 1, 1);
}

//Odbiór wiadomości (timeout)
void MRecvTout(double tout)
{
	struct timeval timeout);
	timeout.tv_sec = int(tout);
	timeout.tv_usec = int(tout % 1 / CLOCKS_PER_SEC * 1000000);
	if (pvm_trecv(-1, MSG_SLV, &timeout) > 0)
		Receives();
}

//Odbiór wiadomości (blokujący)
void MRecv()
{
	pvm_recv(-1, MSG_SLV);
	Receives();
}

//Wysyłanie komunikatu zwrotnego do mastera (do testów)
int feedback;
int maserTID;
void SendToMaster()
{
	if (feedback == FEEDBACK_ON)
	{
		PrepareMessage();
		pvm_send(maserTID, MSG_SLV);
	}
}

//Wysyłanie wiadomości do legionów
/*
	tid		-	tid Legionu, fo którego wysyłany jest komunikat
	type	-	typ komunikatu
	t		-	jakiego traktu dotyczy wiadomość
*/
void SendMessage(int tid, char type, int t)
{
	msgOut.pID = legion.pID;
	msgOut.type = type;
	msgOut.t = t;

	switch (type)
	{
		//Ubieganie o trakt
	case MSG_REQUEST:
		msgOut.iD = myTrakt.priority;
		PrepareMessage();
		pvm_bcast(GRPNAME, MSG_SLV);
		break;

		//Wysłanie liczby legionistów na trakcie
	case MSG_ANSWER:
		if (myTrakt.n == t)
			msgOut.iD = legion.r;
		else
			msgOut.iD = 0;
		
		PrepareMessage();
		pvm_send(tid, MSG_SLV);
		break;

		//Opuszcza trakt
	case MSG_LEAVE:
		msgOut.iD = legion.r;

		//wysłanie do mastera
		SendToMaster();
		//wysłanie do wszystkich
		PrepareMessage();
		pvm_bcast(GRPNAME, MSG_SLV);
		break;
	}
}

//Reakcja na wiadomość
void Receives() {
	//todo
	UnpackMessage();
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

//Wchodzenie na trakt
void Enter() {
	//todo
	state = STARTS;
	priority = trakts[myTrakt].iD;
	trakts[myTrakt].iD++;

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

//Przjeście traktem
void Go() {
	//todo
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

//Odpoczynek
void Rest() {
	myTrakt.n = -1;
	legion.state = WAITS;
	
	//Odczekanie losowego czasu - jeśli pojawi się jakaś wiadomość, to ją odbierz
	double restt = double(rand()%5 + 1);
	time_t start, stop;
	while (restt > 0)
	{
		start = time();
		//odbieranie wiadomości
		MRecvTout(restt);
		stop = time();
		restt -= ((double)(stop - start)) / CLOCKS_PER_SEC;
	}
}

main() {
	srand(time(NULL));
	int i;
	legion.pID = pvm_mytid();
	
	//Pobranie informacji o feedback
	pvm_recv(-1, MSG_MSTR);
	pvm_upkint(&feedback, 1, 1);
	pvm_upkint(&maserTID, 1, 1);
	
	//Pobranie informacji o legionie
	pvm_recv(-1, MSG_MSTR);
	pvm_upkint(&legion.r, 1, 1);
	legion.state = STARTS;

	//pobranie informacji o traktach
	for (i = 0; i < T; i++){
		pvm_recv(-1, MSG_MSTR);
		pvm_upkint(&trakts[i].t, 1, 1);
		pvm_upkint(&trakts[i].time, 1, 1);
		trakts[i].ID = 0;
	}
	
	//dołączenie do grupy i bariera
	pvm_joingroup(GRPNAME);
	pvm_barrier(GRPNAME, L);
	
	//działanie
	while (1)
	{
		Enter();
		Go();
		Rest();
	}

	pvm_exit();					//Opuszczenie maszyny wirtualnej przez mastera
}
