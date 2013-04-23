#include "simulation.h"

#if	!defined(BASIC_TX_RADIUS)
#define	BASIC_TX_RADIUS			60	// metres
#endif

#if	!defined(BEACON_FREQ)
#define	BEACON_FREQ			100000  //send beacon frame 10 times/sec
#endif

#if	!defined(CLIENT_CLEANUP_FREQ)
#define	CLIENT_CLEANUP_FREQ		10000000
#endif

#if	!defined(MAX_CLIENTS_PER_AP)
#define	MAX_CLIENTS_PER_AP		3
#endif

extern	void	init_accesspoint(void);
