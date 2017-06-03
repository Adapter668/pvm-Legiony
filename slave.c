#include "def.h"

struct SMyTrakt{
	int t;			//Nr traktu, którym legion ma przejść
	int priority;	//Priorytet, z jakim proces ubiega się o trakt
	int sum;		//Suma legionistów obecnych na trakcie
	int barrier;		//Na ile odpowiedzi czeka proces
}myTrakt;

struct SLegion legion;
struct STrakt trakts[T];

struct STids{
	int tID[L];
	int n;
}waiting;

void WaitingAdd(int tID)
{
	waiting.tID[waiting.n] = tID;
	waiting.n++;
}

void WaitingClear()
{
	int i;
	for (i = 0; i < waiting.n; i++)
		waiting.tID[waiting.n] = -1;
	waiting.n = 0;
}

//Przygtowywanie wiadomości do wysłania
struct SMessage msgIn, msgOut;
void PrepareMessage()
{
	pvm_initsend(PvmDataDefault);
	pvm_pkbyte(&msgOut.type, 1, 1);
	pvm_pkint(&msgOut.tID, 1, 1);
	pvm_pkint(&msgOut.t, 1, 1);
	pvm_pkint(&msgOut.iD, 1, 1);
}

//Wysyłanie komunikatu zwrotnego do mastera (do testów)
int feedback;
int masterTID;
void SendToMaster()
{
	switch(feedback)
	{
	case FEEDBACK_ON:
		PrepareMessage();
		pvm_send(masterTID, MSG_SLV);
		break;
	
	case FEEDBACK_ON_NO_A:
		if(msgOut.type != MSG_ANSWER)
		{
			PrepareMessage();
			pvm_send(masterTID, MSG_SLV);
		}
		break;
	
	case FEEDBACK_ON_TRAKT:
		if(msgOut.type == MSG_ON_TRAKT || msgOut.type == MSG_LEAVE)
		{
			PrepareMessage();
			pvm_send(masterTID, MSG_SLV);
		}
		break;
	
	case FEEDBACK_OFF:
		break;
	
	default:
		break;
	}
}

//Wysyłanie wiadomości do legionów
/*
type	-	typ komunikatu
t		-	jakiego traktu dotyczy wiadomość
tid		-	procesu, do którego ma zostać wysłana wiadomość
*/
void SendMessage(char type, int t, int tid)
{
	msgOut.tID = legion.tID;
	msgOut.type = type;
	msgOut.t = t;

	switch (type)
	{
		//Ubieganie o trakt
	case MSG_REQUEST:
		msgOut.iD = myTrakt.priority;
		PrepareMessage();
		SendToMaster();
		pvm_bcast(GRPNAME, MSG_SLV);
		break;

		//Wysłanie liczby legionistów na trakcie
	case MSG_ANSWER:
		//Legion jest na trakcie
		if (myTrakt.t == t && legion.state == ON_TRAKT)
		{
			msgOut.iD = legion.r;
			PrepareMessage();
			SendToMaster();
			if(tid > 0)
			{
				pvm_send(tid, MSG_SLV);
			}
			else
			{
				if(waiting.n > 0)			
				{
					pvm_mcast(waiting.tID, waiting.n, MSG_SLV);
				}	
			}
		}
		else
		{
			msgOut.iD = 0;
			PrepareMessage();
			SendToMaster();
			pvm_send(tid, MSG_SLV);
		}
		break;

		//Opuszcza trakt
	case MSG_LEAVE:
		msgOut.iD = legion.r;

		//wysłanie do mastera
		SendToMaster();
		//wysłanie do wszystkich z listy waiting
		if(waiting.n > 0)			
		{
			PrepareMessage();
			pvm_mcast(waiting.tID, waiting.n, MSG_SLV);
		}
		break;

		//Poinformowanie mastera o wejściu na trasę
	case MSG_ON_TRAKT:
		msgOut.iD = legion.r;
		SendToMaster();
		break;

	default:
		break;
	}
}

/*Odpakowanie wiadomości 
Wiadomośc musi być już w buforze odbiorczym!- polecenie pvm_recv lub pvm_trecv przed wywołaniem procedury*/
void UnpackMessage()
{
	pvm_upkbyte(&msgIn.type, 1, 1);
	pvm_upkint(&msgIn.tID, 1, 1);
	pvm_upkint(&msgIn.t, 1, 1);
	pvm_upkint(&msgIn.iD, 1, 1);
}

//Reakcja na wiadomość
void Receives() {
	//Odpakowanie wiadomości
	UnpackMessage();

	//Żądanie
	if (msgIn.type == MSG_REQUEST)
	{
		if (trakts[msgIn.t].iD <= msgIn.iD)
			trakts[msgIn.t].iD = msgIn.iD + 1;

		if (msgIn.t == myTrakt.t)
		{
			if (legion.state == ON_TRAKT) {
				WaitingAdd(msgIn.tID);
				SendMessage(MSG_ANSWER, myTrakt.t, msgIn.tID);
			}

			if (legion.state == STARTS)
			{
				if (msgIn.iD < myTrakt.priority || (msgIn.iD == myTrakt.priority && msgIn.tID < legion.tID))
				{
					SendMessage(MSG_ANSWER, myTrakt.t, msgIn.tID);
				}
				else
				{
					WaitingAdd(msgIn.tID);
				}
			}
		}
		else
			SendMessage(MSG_ANSWER, msgIn.t, msgIn.tID);
	}

	//Opuszczanie trasy
	if (msgIn.type == MSG_LEAVE)
	{
		if (msgIn.t == myTrakt.t)
		{
			myTrakt.sum -= msgIn.iD;
		}
	}

	//Odpowiedź
	if (msgIn.type == MSG_ANSWER)
	{
		if (msgIn.t == myTrakt.t)
		{
			myTrakt.sum += msgIn.iD;
			myTrakt.barrier--;
		}
	}
}

//Odbiór wiadomości (timeout)
void MRecvTout(int tout)
{
	time_t start, stop;
	while (tout > 0)
	{
		start = time(NULL);	
		struct timeval timeout;
		timeout.tv_sec = (int) tout;
		timeout.tv_usec = (int) ((tout - floor(tout)) / 1000000);
		
		//odbieranie wiadomości
		if (pvm_trecv(-1, MSG_SLV, &timeout) > 0)
			Receives();

		stop = time(NULL);	
		tout -= stop - start;
	}
}

//Odbiór wiadomości (blokujący)
void MRecv()
{
	pvm_recv(-1, MSG_SLV);
	Receives();
}

//Wchodzenie na trakt
void Enter() {
	//Zmiana stanu
	legion.state = STARTS;
	
	//Ustawienie zmiennych traktu, o który legion będzie się ubiegał
	myTrakt.t = rand() % T;
	myTrakt.priority = trakts[myTrakt.t].iD++;
	myTrakt.sum = 0;
	myTrakt.barrier = L - 1;

	//Wysłanie żadania 
	SendMessage(MSG_REQUEST, myTrakt.t, -1);

	//Oczekiwanie na wszystkie odpowiedzi
	while (myTrakt.barrier > 0)
		MRecv();
	
	//Jeśli trasa przepełniona, to oczekiwanie na jej zwolnienie
	while (myTrakt.sum + legion.r > trakts[myTrakt.t].t)
		MRecv();
}

//Przjeście traktem
void Go() {
	//Zmiana stanu
	legion.state = ON_TRAKT;

	//Wysłanie wiadomości o wejściu na trakt do mastera
	SendMessage(MSG_ON_TRAKT, myTrakt.t, -1);
	
	//Wysłanie wiadomości Answer do oczekujących legionów
	SendMessage(MSG_ANSWER, myTrakt.t, -1);

	//Spędzenie czasu na trakcie
	MRecvTout(trakts[myTrakt.t].time);

	//Zejście z traktu
	SendMessage(MSG_LEAVE, myTrakt.t, -1);

	//Wyczyszczenie tablicy waiting
	WaitingClear();
}

//Odpoczynek
void Rest() {
	myTrakt.t = -1;
	legion.state = WAITS;
	
	//Odczekanie losowego czasu - jeśli pojawi się jakaś wiadomość, to ją odbierz
	MRecvTout(rand()%5 + 1);
}

main() {
	
	WaitingClear();

	int i;
	legion.tID = pvm_mytid();
	srand(legion.tID);
	
	//Pobranie informacji o feedback
	pvm_recv(-1, MSG_MSTR);
	pvm_upkint(&feedback, 1, 1);
	pvm_upkint(&masterTID, 1, 1);
	
	//Pobranie informacji o legionie
	pvm_recv(-1, MSG_MSTR);
	pvm_upkint(&legion.r, 1, 1);
	legion.state = STARTS;

	//pobranie informacji o traktach
	for (i = 0; i < T; i++){
		pvm_recv(-1, MSG_MSTR);
		pvm_upkint(&trakts[i].t, 1, 1);
		pvm_upkint(&trakts[i].time, 1, 1);
		trakts[i].iD = 0;
	}
	
	//dołączenie do grupy i bariera
	pvm_joingroup(GRPNAME);
	pvm_barrier(GRPNAME, L);
	
struct SMessage asdf;
asdf.tID = legion.tID;
		asdf.t = -1;
		asdf.iD = 11111;
	//działanie
	while (1)
	{
		Enter();
		Go();
		Rest();
	}

	pvm_exit();					//Opuszczenie maszyny wirtualnej przez slavea
}
