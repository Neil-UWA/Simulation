#include <cnet.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "common.h"
#include "client.h"
#include "coverage.h"
#include "mapping.h"

static	bool		foundAP		=	false;  // found ap?
static	bool		associated	=	false;  // associated with an ap?
static	int			targetAP	=	NULLAP; // the ap talk to  
static	int			times		=	5;		// listen for 5 seconds
static	AP			*apList		=	NULL;   // a list to store aps
static	int			count		=	0;		// counter of apList
static  double		mapscale;

static	CnetTimerID	timeout_tid	=	NULLTIMER;
static	CnetTimerID	talk_tid	=	NULLTIMER; 
static	CnetTimerID	ass_tid		=	NULLTIMER; 

/**
* @brief compare beacon frames, whether they are the same
*
* @param frame
* @param rxsignal
* @return True indicates the AP hasn't been stored in the APList 
*/
bool 
compareAP(FRAME frame, double rxsignal){
	bool duplicated = false; // AP has been stored? 

	for (int i = 0; i < count; i++) {
		if (frame.nodeinfo.nodenumber == apList[i].AP_addr) {
			apList[i].rxsignal = rxsignal; // update signal 
			duplicated = true;
		}
	}

	return duplicated;
}


/**
* @brief add APs to the APList with their signal strength
*
* @param frame
* @param rxsignal
*/
void 
addToAPList(FRAME frame, double rxsignal)
{
	if (count == 0) {
		apList = (AP*)realloc(apList, (count+1)*sizeof(AP));

		apList[count].AP_addr = frame.nodeinfo.nodenumber;
		apList[count].rxsignal = rxsignal ;
		count += 1;
	}
	else {
		if (!compareAP(frame, rxsignal)) {
			apList = realloc(apList, (count+1)*sizeof(AP));

			apList[count].AP_addr = frame.nodeinfo.nodenumber;
			apList[count].rxsignal = rxsignal ;
			count += 1;
		}							
	}		

}

/**
* @brief Decide which AP has the strongest signal
*
* @return  -1 indicates error, otherwise return the AP address
*/
int	
whichAP(void){
	AP temp;

	if (apList) {
		memcpy(&temp, &apList[0], sizeof(AP));

		for (int i = 1; i < count; i++) {
			if(abs(temp.rxsignal) >= abs(apList[i].rxsignal)){
				memcpy(&temp, &apList[i], sizeof(AP));
			} 
		}
	}
	else {
		return -1;
	}

	return temp.AP_addr;
}

/**
* @brief show the  ap list. This function will be commented later
*/
#ifdef DEBUG
void 
show(void){
	if (count == 0) 
		printf("no APs found\n");
	else {
		for (int i = 0; i < count; i++) {
			printf("AP number: %d, received signal: %lf, total number of APs found now: %d\n", 
					apList[i].AP_addr, apList[i].rxsignal, count);
		}
	}
}
#endif

void
disconnect(int	dst){
	FRAME	frame = initFrame(DL_DISCONNECT, dst, "DISCONNECT");
	size_t	length = sizeof(FRAME);

	CHECK(CNET_write_direct(dst, (FRAME *)&frame, &length));
	#ifdef DEBUG
	printf("sent disconnect frame to the associated AP %d\n", dst);
	#endif

	associated	=	false;
}


EVENT_HANDLER(talking)
{
	CnetPosition	current;	
	
	mapscale = CNET_get_mapscale();
	CHECK(CNET_get_position(&current, NULL)); 

	if (!foundAP) {
		#ifdef DEBUG
		printf("no signal here\n");
		#endif
	    TCLTK("$map create rect %d %d %d %d -width 1 -outline %s -fill %s",
		    SCALE(current.x), SCALE(current.y), SCALE(current.x + 1), SCALE(current.y +1),
		    COLOUR_OBJECTS, "red");
	}


	if (foundAP&&!associated) {
		printf("haven't associated with an AP!\n");
	}

	// the client is doing nothing, so we reset all flags
	if (CNET_carrier_sense(1) == 0) {
		if (foundAP) {
			foundAP =false;
		}

		if (associated) {
			associated	= false;
			count		=	0;
			targetAP	=	NULLAP;
		}
	}

	talk_tid = CNET_start_timer(EV_TALKING, 1000000, 0);
}

void 
init_talking(void)
{
	CHECK(CNET_set_handler(EV_TALKING, talking, 0));
}

void 
start_talking(void)
{
	talk_tid = CNET_start_timer(EV_TALKING, 1000000, 0);
}

void 
stop_talking(void)
{
	CNET_stop_timer(talk_tid);
}

EVENT_HANDLER(timeouts)
{
	if (associated) {
		printf("timeouts, lost connection with associated AP!\n");

		disconnect(targetAP);

	}

	if (foundAP&&!associated) {
		printf("NO response from the AP %d\n", targetAP);
		foundAP = false;
	}
		init_searchAP();
		start_talking();
}


void 
init_timeout(void)
{
	CHECK(CNET_set_handler(EV_TIMEOUT, timeouts, 0));
}

void 
start_timeout(void)
{
	timeout_tid = CNET_start_timer(EV_TIMEOUT, 2000000, 0);
}

void 
stop_timeout(void)
{
	CNET_stop_timer(timeout_tid);
}

EVENT_HANDLER(associate_with_ap)
{

	if (CNET_carrier_sense(1) == 0) {
		if (foundAP && !associated) {
			transmit(DL_RTS, targetAP, "RTS", data);
		}
	}

	ass_tid	=	CNET_start_timer(EV_ASSOCIATE, 1000000, data);
}

void
init_association(double data)
{
	CHECK(CNET_set_handler(EV_ASSOCIATE, associate_with_ap, data));
}

void
start_association(void)
{
	ass_tid = CNET_start_timer(EV_ASSOCIATE, 1000000, 0);
}

void
stop_association(void)
{
	CNET_stop_timer(ass_tid);
}

EVENT_HANDLER(talk_to_ap)
{
	FRAME	frame	=	initFrame(DL_DATA, targetAP, "DATA");
	size_t	length	=	sizeof(FRAME);
	int		link	=	1;

	CHECK(CNET_write_physical_reliable(link, (FRAME *)&frame, &length));

	printf("talking to ap %d\n", targetAP);

	start_timeout();
}

/**
* @brief listen to frames which come from target AP (the associated one)
*/
EVENT_HANDLER(listen_to_ap)
{
	FRAME	frame;
	size_t	length	=	sizeof(FRAME);
	int		link;
	double	rxsignal;
	CnetPosition	current;

	CHECK(CNET_get_position(&current, NULL)); //get my current location	
	CHECK(CNET_read_physical(&link, (FRAME *)&frame, &length));
	CHECK(CNET_wlan_arrival(link, &rxsignal, NULL));

	frame.rxsignal	=	rxsignal;

	// frames from APs
	if (frame.nodeinfo.nodenumber == targetAP && frame.dst == nodeinfo.nodenumber) {
		if (foundAP&&!associated) {

			// acknowlege the AP that the CTS frame has been received
			if (frame.kind == DL_CTS)
			{
				stop_timeout();
				transmit(DL_ASSOCIATION_ACK, targetAP, "ASSOCATION_ACK", rxsignal);
			}

			// if received a ASSOCATION_ACK frame from the targetAP, then the association is completed
			if (frame.kind == DL_ASSOCIATION_ACK)
			{
				associated	=	true;

				CHECK(CNET_set_handler(EV_TIMER3, talk_to_ap, 0));			
				CNET_start_timer(EV_TIMER3, 1000000, 0);
			}
		}
		else {
			if (frame.kind == DL_ACK) {
				stop_timeout();
				CNET_start_timer(EV_TIMER3, 1000000, 0);
			}
		}

		//set_covered(current);
		cell state;
		state.current = current;
		state.cellState = COVERED;	
		size_t	len = sizeof(cell);
		CHECK(CNET_write_direct(0,(cell *)&state,&len ));

#ifdef DEBUG
		showFrame(frame);	
		#endif
	}

}

EVENT_HANDLER(searching_ap)
{
	FRAME	frame;
	size_t	length	=	sizeof(FRAME);
	int		link;
	double	rxsignal;
	CnetPosition	current;

	CHECK(CNET_get_position(&current, NULL)); //get my current location	
	CHECK(CNET_read_physical(&link, (FRAME *)&frame, &length));
	CHECK(CNET_wlan_arrival(link, &rxsignal, NULL));

	frame.rxsignal	=	rxsignal;


	// here, we are only interested in beacon frames from access points
	if (frame.nodeinfo.nodetype	==	NT_ACCESSPOINT) {

		// found ap signals
		if (frame.kind == DL_BEACON && !foundAP) {
			stop_talking();

			if (times != 0) {
				printf("AP signals have been found, searching AP with the strongest signal\n");
				if (!frame.overload) {
					addToAPList(frame, rxsignal);
				}
				times --;	
				#ifdef DEBUG
				show();
				#endif
			}
			else {
				foundAP	= true; //mark that we have found APs around the client
				times = 5; //reset the counter
				targetAP = whichAP(); //this is the AP with the strongest signal
				
				CHECK(CNET_set_handler(EV_PHYSICALREADY, listen_to_ap, 0));

				//start the association process 
				transmit(DL_RTS, targetAP, "RTS", rxsignal);	

				init_timeout();
				start_timeout();
			}
		}

		//set_covered(current);
	}
}

void
init_searchAP(void){
	CHECK(CNET_set_handler(EV_PHYSICALREADY, searching_ap, 0));
}
