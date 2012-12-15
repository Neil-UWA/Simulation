/** 
 * TIMERS 
 * Timers from 1 - 4 belong to clients
 * TImers from 5 - 8 belong to access points
 */
#define EV_BEACON	EV_TIMER1
#define	EV_RECEIVE  EV_TIMER2

#define	EV_TALKING	EV_TIMER5
#define EV_LISTEN	EV_TIMER6

#define	FREQUENCY	10000 //the frequence sending a beacon frame
#define MAX_SIZE 50

typedef struct{
	CnetNodeInfo	nodeinfo;
	CnetPosition	current;
	char			message[MAX_SIZE];
}FRAME;
                    

extern FRAME initFrame(char *message, CnetPosition current);

/** sense message in the air */
extern EVENT_HANDLER(listening);
