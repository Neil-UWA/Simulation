#include <cnet.h>
#include <stdlib.h>
#include "common.h"
#include "accesspoint.h"
#include "coverage.h"

#define	MAX_CLIENTS	(5) //the maximun clients an AP can handle

static	CnetTimerID	beacon_tid		=	NULLTIMER;
static	int			total_clients	=	0; //the total number of clients associated with 

/**
* @brief determine whether the maximun clients number is reached
*
* @return 
*/
bool 
get_overload(){
	return total_clients == MAX_CLIENTS;
}

/**
* @brief  sending beacon frames 10 times every second
*/
EVENT_HANDLER(beaconning)
{
	FRAME	frame	=	initFrame(DL_BEACON, BROADCAST, "BEACON");
	size_t	length	=	sizeof(FRAME);
	int		link	=	1;

	frame.overload	=	get_overload();
	
	if (CNET_carrier_sense(1)==0) {
		CHECK(CNET_write_physical_reliable(link, (FRAME *)&frame, &length));
		#ifdef DEBUG
		printf("\nSENDING BEACONS\n");
		#endif
	}
	
//	if (nodeinfo.nodenumber == 0) {
//		printf("%lf\n", coverage_rate());
//	}
	beacon_tid = CNET_start_timer(EV_BEACON, FREQUENCY + CNET_rand()%1000,	0);
}

void 
init_beacon(void)
{
	CHECK(CNET_set_handler(EV_BEACON, beaconning, 0));	
}

void 
start_beacon(void)
{
	beacon_tid = CNET_start_timer(EV_BEACON, FREQUENCY + CNET_rand()%1000,	0);
}

EVENT_HANDLER(listenning)
{
	FRAME	frame;
	size_t	length = sizeof(FRAME);
	int		link;
	double	rxsignal;

	CHECK(CNET_read_physical(&link, (FRAME *)&frame, &length));
	CHECK(CNET_wlan_arrival(link, &rxsignal, NULL));
	
	frame.rxsignal	=	rxsignal;
	frame.overload	=	get_overload();

	#ifdef DEBUG
	showFrame(frame);
	#endif
	printf("the total associated clients is %d\n", total_clients);

	//this frame is for me and it comes from an mobile client
	if (frame.nodeinfo.nodetype == NT_MOBILE && 
			frame.dst == nodeinfo.nodenumber) {

		switch (frame.kind){
			case DL_RTS:
					transmit(DL_CTS, frame.nodeinfo.nodenumber, "CTS", rxsignal);
				break;

			case DL_ASSOCIATION_ACK:
				transmit(DL_ASSOCIATION_ACK, frame.nodeinfo.nodenumber, "ASSOCIATED", rxsignal);
				total_clients += 1;
				break;	

			case DL_DATA:
				transmit(DL_ACK, frame.nodeinfo.nodenumber, "DATA RECEIVED", rxsignal);
				break;

			case DL_DISCONNECT:		
				total_clients -= 1;
				break;

			default:
				break;
		}
	} 
}
