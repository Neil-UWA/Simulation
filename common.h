#include "mapping.h"
/** 
 * TIMERS 
 * Timers from 1 - 4 belong to clients
 * TImers from 5 - 8 belong to access points
 */
#define EV_BEACON	EV_TIMER1
#define	EV_RECEIVE  EV_TIMER2

#define	EV_TALKING	EV_TIMER5
#define EV_LISTEN	EV_TIMER6

#define	FREQUENCY	100000 //the frequence sending a beacon frame
#define MAX_CLIENT	200 /**< the maximum number of clients that an ap can handle */ 
#define MAX_SIZE 50 /**< the maximum length of a message*/

typedef enum{DL_BEACON, DL_DATA}FRAMEKIND;
typedef struct{
	CnetNodeInfo	nodeinfo;
	CnetPosition	current;
	char			message[MAX_SIZE];
}FRAME;
                    
typedef struct{
	int		nodenumber; /**< the node number of an access point */
	double	signal; /**< signal strength received */
}SIGNAL;

extern FRAME initFrame(char *message, CnetPosition current);

bool compareAP(FRAME frame, double rxsignal);
void addToSignalList(FRAME frame, double rxsignal);
int	whichAP();
void show();

/** sense message in the air */
extern EVENT_HANDLER(listening);
