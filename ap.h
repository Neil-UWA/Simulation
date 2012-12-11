#include    <cnet.h>

#define EV_BEACON	EV_TIMER2

#define	FREQUENCY	10000
#define MAX_SIZE 50

typedef struct{
	char message[MAX_SIZE];
	int  nodenumber;
}PACKAGE;

extern	EVENT_HANDLER(ap);
