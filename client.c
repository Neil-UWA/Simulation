/** 
 * @file client.c	
 * @brief Client Module
 * @author JINANG LU
 * @date 16 Dec 2012
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


/** 
//TODO ASSOCIATE WITH THE AP WITH THE STRONGEST SIGNAL 
*/
EVENT_HANDLER(client_talking){
	FRAME	frame;
	size_t	length = sizeof(FRAME);
	CnetPosition	current;
	char	*message = "hello";

	CHECK(CNET_get_position(&current, NULL));
	frame = initFrame(message, current);

	printf("nodename %s\t", frame.nodeinfo.nodename);
	printf("nodetime %lld\n", frame,nodeinfo.time_of_day.usec);
	CHECK(CNET_write_physical_reliable(1, (FRAME *)&frame, &length));

	CNET_start_timer(EV_TALKING, (CnetTime) 1000000, 0);
}

