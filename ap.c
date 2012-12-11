#include    <string.h>

#include    "ap.h"

EVENT_HANDLER(ap){
  char	*beacon	= "this is a beacon";
  PACKAGE frame;
  size_t	len = sizeof(frame);

  frame.nodenumber = nodeinfo.nodenumber;
  strcpy(frame.message, beacon);
  printf("AP %d beaconing\n", frame.nodenumber);

  CNET_write_physical_reliable(1,&frame,&len);
  CNET_start_timer(EV_BEACON, (CnetTime) FREQUENCY, 0);
}
