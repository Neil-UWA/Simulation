/** 
 * @file client.c	
 * @brief Client Module
 */
#include <time.h>
#include <cnet.h>
#include "client.h"   
#include "common.h"


#define		CURRENT		nodeinfo.time_of_day.usec

//static void sendRTS(){ }
//
//static void receiveCTS(){ }
//
//static void associateAP(){ }


/** ASSOCIATE WITH THE AP WITH THE STRONGEST SIGNAL */
EVENT_HANDLER(client){
	FRAME	frame;
	size_t	length = sizeof(FRAME);
	CnetPosition	current;
	char	*message = "hello";

	CHECK(CNET_get_position(&current, NULL));
	frame = initFrame(message, current);

	printf("nodename %s\t nodetime: %lld\n", frame.nodeinfo.nodename, frame.nodeinfo.time_in_usec);
	CHECK(CNET_write_physical_reliable(1, (FRAME *)&frame, &length));

	CNET_start_timer(EV_TALKING, (CnetTime) 1000000, 0);
}
