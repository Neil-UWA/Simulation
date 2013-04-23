#ifndef WIDTH && HEIGHT 
#define WIDTH	(600)   // MAP WIDTH 
#define HEIGHT	(1200)  // MAP HEIGHT
#endif

#define	COMMENT		'#'
#define	COLOUR_OBJECTS	"#aaaaaa"
#define	COLOUR_PATH	"#80CC80"

// defualt values for transmitter and receiver
#define tx_frequency_GHz (2.45)	
#define tx_power_dBm (14.771)		
#define	tx_antenna_gain_dBi (2.14)
#define tx_cable_loss_dBm (0.0)	
#define rx_antenna_gain_dBi (1.14)
#define rx_cable_loss_dBm (0.0)	
#define rx_sensitivity_dBm (-82.0)  

#ifndef RADIUS
#define RADIUS (60)
#endif
#define RADIUS2	(RADIUS*RADIUS)

#define FOR_LOOP	for(i=0;i<WIDTH;i++)for(j=0;j<HEIGHT;j++)

typedef enum {
	FREE = 0,
	OBSTACLE,
	PATH,
	COVERED,
	AP
} STATE;

typedef struct _LOCATION {
	int	x;
	int	y;
	int coverage;
} LOCATION;     

typedef enum {
    T_OBJ	= 0,
    T_TEXT,
    T_POINT,
    T_PATH
} OBJTYPE;

typedef struct {
    OBJTYPE	type;
    char	*str;
    int		x0;
    int		y0;
    int		x1;
    int		y1;
} OBJECT;

//-------------------------------------------------------------//
//					FOR GA SIMULATION		   				   //
//-------------------------------------------------------------//
#ifndef P_MUTATION
#define P_MUTATION 0.1 		//probability of mutation
#endif

#ifndef	P_CROSSOVER
#define	P_CROSSOVER 0.6		//probability of crossover
#endif

#ifndef MAX_POP
#define MAX_POP 300			// the number of maximum population
#endif 

#ifndef MAX_ITERATION			
#define MAX_ITERATION 100	// the number of maximum iteration
#endif

#ifndef NUM_AP					 	
#define NUM_AP 50			// the number of maximum APs
#endif

typedef struct _INDIVIDUAL {
	LOCATION	locations[NUM_AP];
	double		fitness;			//how many path (pixes) are covered
	double		relative_fitness;	//fitness/total_fitness
} INDIVIDUAL;
