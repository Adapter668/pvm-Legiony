#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pvm3.h>
#include <signal.h>
#include <math.h>  

#define SLAVENAME "slave"

//T < L!
#define L 10 //Liczba legionów
#define T 5 //Liczba traktów

#define MSG_MSTR 1
#define MSG_SLV 2

#define WAITS 3				//Legion odpoczywa
#define STARTS 4			//Legion czeka na możliwość wejścia na trakt
#define ON_TRAKT 5			//Legion porusza się traktem

#define MSG_REQUEST 'R'		//Żądanie o podanie informacji ile legionistów znajduje się na trasie
#define MSG_ANSWER 'A'		//Odpowiedź na żadanie
#define MSG_LEAVE 'L'		//Informacja o opuszczeniu traktu
#define MSG_ON_TRAKT 'T'	//Informacja o wejściu na trakt (tylko do mastera)

#define GRPNAME "Legions"	//Nazwa grupy dla slaveów

//Czy slave ma wysyłać wiadomości do mastera 
#define FEEDBACK_ON 1		//Wyślij wszystkie wiadomości, które wysyłasz pozostałym 
#define FEEDBACK_OFF 0		//Nie wysyłaj wiadomości
#define FEEDBACK_ON_NO_A 2	//Wyślij wyszystko, pomiń wiadomości A
#define FEEDBACK_ON_TRAKT 3	//Wysyłaj tylko wiadomości T i L

struct STrakt {
	int t;		//maksymalna liczba legionistów, jaka może się poruszać danym traktem
	int time;	//czas przejścia przez dany trakt (s)
	int iD;		//aktualny priorytet jaki dostanie proces, jeśli będzie ubiegał się o trakt
};		

struct SMessage {
	char type; 	//R [Request], A [Answer], L [Leave]
	int tID;	//kto pisze
	int t;		//numer traktu jakiego dotyczy wiadomość
	int iD;		//jeśli type==R to z jakim numerem ubiegam się o trakt; A lub L liczba legionistów w legionie
};

struct SLegion {
	int tID;	//Numer PID procesu (nr legionu)
	int r;		//liczba legionistów w legionie procesu
	int state;	//Stan (WAITS, STARTS, ON_TRAKT)
};

