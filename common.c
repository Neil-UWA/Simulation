/** 
 * @file common.c	
 * @brief Common functions that used by others
 */

#include <cnet.h>  

#include "common.h"
#include "string.h"

/** initialize the data structure */
FRAME initFrame(char *message, CnetPosition current){
	FRAME	frame;

	memset(&frame, 0, sizeof(FRAME));

	strcpy(frame.message, message);
	frame.nodeinfo = nodeinfo;
	frame.current	= current;
	
	return frame;
}

/** sense message in the air */
EVENT_HANDLER(listening){
	int		link;
	FRAME	frame;
	size_t	length = sizeof(FRAME);
	double	rxsignal;

	CHECK(CNET_read_physical(&link, (FRAME *) &frame, &length));
	CHECK(CNET_wlan_arrival(link, &rxsignal, NULL));

	fprintf(stdout, "from node: %d\t signal: %lf, \tmessage: %s\n", frame.nodeinfo.nodenumber, rxsignal, frame.message);  	
}        
