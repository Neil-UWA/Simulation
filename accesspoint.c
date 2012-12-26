/** 
 * @file accesspoint.c	
 * @brief Acess Point Module
 */

#include <cnet.h>
#include <string.h>

#include "accesspoint.h"
#include "walking.h"
#include "common.h"

//static int	total_clients	=	0; /**< number of clients connected */

//static void receiveRTS(){ }
//
//static void sendCTS(){ }
//
//static void associateClient(){ }

/**
 * @brief sending beacon frames
 */
EVENT_HANDLER(beaconing){
	int			link = 1;
	FRAME		frame;
	size_t		length = sizeof(frame);
	CnetPosition current;
		
	memset(&frame, 0, sizeof(FRAME));

	CHECK(CNET_get_position(&current, NULL));
	frame = initFrame("beacon", current);

	CHECK(CNET_write_physical_reliable(link, (FRAME *)&frame, &length));
	CNET_start_timer(EV_BEACON, FREQUENCY, 0);
}
