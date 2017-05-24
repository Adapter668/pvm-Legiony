#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <pvm3.h>

#define SLAVENAME "slave"

#define L 10
#define T 5

#define MSG_MSTR 1
#define MSG_SLV 2

#define WAITS 3
#define STARTS 4
#define ON_TRAKT 5

struct STrakt {
	int t;		//maksymalna liczba legionistów, jaka może się poruszać danym traktem
	int time;	//czas przejścia przez dany trakt (ms)
	int ID;		//aktualny priorytet jaki dostanie proces, jeśli będzie ubiegał się o trakt
}trakts[T];		//lista traktów

struct SMessage {
	char type; 	//R [Request], A [Answer], L [Leave]
	int PID;	//kto pisze
	int T;		//numer traktu jakiego dotyczy wiadomość
	int ID;		//jeśli type==R to z jakim numerem ubiegam się o trakt, A lub L liczba legionistów w legionie
};
