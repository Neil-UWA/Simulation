#define	FREQUENCY	(100000) //send beacon frame 10 times/sec
#define	BROADCAST	(-1) //broadcasting 
#define	MAXSIZE		(50)
#define	NULLAP		(-1) //no ap exists

#define EV_BEACON		EV_TIMER1
#define EV_TALKING		EV_TIMER2
#define	EV_ASSOCIATE	EV_TIMER4
#define	EV_TIMEOUT		EV_TIMER5


typedef struct {
    char	*text;
	double 	x0;
   	double 	y0;
   	double 	x1;
   	double 	y1;
} OBJECT;

/**
* @brief types of frame
*/
typedef enum {
	DL_BEACON,
	DL_CTS,
	DL_CTS_ACK,
	DL_RTS,		
	DL_RTS_ACK,
	DL_ASSOCIATION_ACK,
	DL_DATA,
	DL_ACK,
	DL_DISCONNECT,
	DL_OVERLOAD
} KIND; 

typedef struct _FRAME {
	KIND			kind;
	int				dst;	//	the target ap/client talking to 
	char			msg[MAXSIZE];
	CnetNodeInfo	nodeinfo;
	CnetPosition	position;
	double			rxsignal;
	bool			overload;
} FRAME;

typedef struct _AP {
	int			AP_addr;
	double		rxsignal;
} AP;

typedef struct _cell {
	CnetPosition	current;
	int				cellState; 
} cell;

extern	FRAME	initFrame(KIND kind, int dst, char	*msg);
extern	void	transmit(KIND kind, int dst, char *msg, double rxsignal);
extern	void	showFrame(FRAME frame);
