/** 
 * @file common.c	
 * @brief Common functions that used by others
 */

#include <cnet.h>  
#include <stdlib.h>
#include <string.h>

#include "common.h"

static	SIGNAL *signal_list	=	NULL; /**< store signal strength of APs */
static	int		count	= 0; /**< counts for signal_list */

/** initialize the data structure */
FRAME initFrame(char *message, CnetPosition current){
	FRAME	frame;

	memset(&frame, 0, sizeof(FRAME));

	strcpy(frame.message, message);
	frame.nodeinfo = nodeinfo;
	frame.current	= current;
	
	return frame;
}

/**
 * @brief whether the node with the same number has been stored
 *
 * @param FRAME frame
 *
 * @return bool True if it has been stored. Otherwise, return False 
 */
bool compareAP(FRAME frame, double rxsignal){
	bool duplicated = false; // AP has been stored? 

	for (int i = 0; i < count; i++) {
		if (frame.nodeinfo.nodenumber == signal_list[i].nodenumber) {
			signal_list[i].signal = rxsignal; // update signal 
			duplicated = true;
		}
	}

	return duplicated;
}

/**
* @brief store the accessponts have been found with their signal strngth
* this will not update the signal strength of APs dynamically
*
* @param FRAME frame
* @param double rxsignal
*/
void addToSignalList(FRAME frame, double rxsignal){
	if (frame.nodeinfo.nodetype == NT_ACCESSPOINT) {

		if (count == 0) {
			signal_list = (SIGNAL*)realloc(signal_list, (count+1)*sizeof(SIGNAL));
			signal_list[count].nodenumber = frame.nodeinfo.nodenumber;
			signal_list[count].signal = rxsignal ;
			count += 1;
		}else {
			if (!compareAP(frame, rxsignal)) {
				signal_list = realloc(signal_list, (count+1)*sizeof(SIGNAL));
				signal_list[count].nodenumber = frame.nodeinfo.nodenumber;
				signal_list[count].signal = rxsignal ;
				count += 1;
			}							
		}		

	}
}

/**
* @brief decide which AP has the strongest signal 
*
* @return int -1 if no APs found. Otherwise return the nodenumber 
* with the strongest signal
*/
int	whichAP(){
	SIGNAL temp;

	if (signal_list) {
		memcpy(&temp, &signal_list[0], sizeof(SIGNAL));
		for (int i = 1; i < count; i++) {
			if(abs(temp.signal) >= abs(signal_list[i].signal)){
				memcpy(&temp, &signal_list[i], sizeof(SIGNAL));
			} 
		}
	}else {
		return -1;
	}

	return temp.nodenumber;
}

//TODO will be commentted later, just for checking purpose
void show(){
	int	node;
	for (int i = 0; i < count; i++) {
		printf("node number: %d, received signal: %lf, count: %d\n", signal_list[i].nodenumber, signal_list[i].signal, count);
	}

node =	whichAP();

	printf("ap %d\n", node);
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
	
	if (nodeinfo.nodetype == NT_MOBILE) {
		addToSignalList(frame, rxsignal);
		show();
	}
}        
