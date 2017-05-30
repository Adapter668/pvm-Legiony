#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pvm3.h>
#include <signal.h>
#include <sys/time.h>
#include <math.h>  

#define SLAVENAME "slave"

//T < L!
#define L 10 //Liczba legionów
#define T 5 //Liczba traktów

#define MSG_MSTR 1
#define MSG_SLV 2

#define WAITS 3
#define STARTS 4
#define ON_TRAKT 5

#define MSG_REQUEST 'R'
#define MSG_ANSWER 'A'
#define MSG_LEAVE 'L'
#define MSG_ON_TRAKT 'T'

#define GRPNAME "Legions"

//Czy slave ma wysyłać wiadomości do mastera o wchodzeniu i schodzeniu z trasy
#define FEEDBACK_ON 1
#define FEEDBACK_OFF 0

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

