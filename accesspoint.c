/** 
 * @file	
 * @brief Acess Point Module
 */

#include <cnet.h>
#include <string.h>

#include "accesspoint.h"
#include "walking.h"
#include "common.h"

//static void receiveRTS(){ }
//
//static void sendCTS(){ }
//
//static void associateClient(){ }

/** receive */
// static EVENT_HANDLER(receive){
//	int			link;
//	FRAME		frame;
//	size_t		length	=	sizeof(FRAME);
// 
//	CNET_read_physical(&link, &frame, &length);
//	printf("Message %s from node %d\n", frame.message, frame.nodenumber );
//	(CNET_start_timer(EV_RECEIVE, FREQUENCY, 0));
// }       



/** sending beacon frames to the air */
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
