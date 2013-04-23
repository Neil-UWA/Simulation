#include "simulation.h"
#include "walking.h"

#if	!defined(DATA_FREQ)
#define	DATA_FREQ			1000000		// transmit every 1 second
#endif

#if	!defined(AP_CLEANUP_FREQ)
#define	AP_CLEANUP_FREQ			5000000		// cleanup APs every 5 seconds
#endif

#if	!defined(ASSOC_CANCEL_PERIOD)
#define	ASSOC_CANCEL_PERIOD		2000000		// cancel association request
#endif

extern	void	init_client(void);
