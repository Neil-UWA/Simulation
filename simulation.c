#include <cnet.h>
#include <time.h>
#include <string.h>
#include "mapping.h"
#include "walking.h"

#define	EV_TALKING	EV_TIMER1
#define EV_BEACON	EV_TIMER2

#define MAX_SIZE 50
//static char message[] = "hello";
static char beacon[] = "I am a beacon frame";

typedef struct{
	char	message[MAX_SIZE];
	//int		nodetype; //TYPE OF THE NODE
	int		nodenumber;
}package;

EVENT_HANDLER(click){
  time_t  now;
  time(&now);
  printf("hello\n");    
  printf("%s\n", ctime(&now));
  CNET_start_timer(EV_TALKING, (CnetTime)2000000, 0);
}

EVENT_HANDLER(ap){
  static package frame;
  size_t	len = sizeof(frame);

	frame.nodenumber = nodeinfo.nodenumber;
	  strcpy(frame.message,beacon);
  printf("I am an accesspoint with one wlan link\n");

  CNET_write_physical_reliable(1,&frame,&len);
  CNET_start_timer(EV_BEACON, (CnetTime)2000000, 0);
}

EVENT_HANDLER(reboot_node){
  char**	argv = (char**) data;

  if(argv[0]){
    readmap(argv[0]);
	
    //FUNCTIONS FOR ACCESS POINTS
    if(nodeinfo.nodetype == NT_ACCESSPOINT) {
      CNET_set_handler(EV_BEACON, ap, 0);
      CNET_start_timer(EV_BEACON, (CnetTime)1000000, 0);
    }

    //FUNCTIONS FOR MOBILE CLIENTS
    if(nodeinfo.nodetype == NT_MOBILE) {
	  init_walking();
	  start_walking();
      CNET_set_handler(EV_TALKING, click, 0);
      CNET_start_timer(EV_TALKING, (CnetTime)1000000, 0);
    }
  }
}
