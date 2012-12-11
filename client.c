#include    <time.h>

#include    "client.h"

EVENT_HANDLER(client){
  time_t  now;
  time(&now);
  printf("hello\n");    
  printf("%s\n", ctime(&now));
  CNET_start_timer(EV_TALKING, (CnetTime)2000000, 0);
}
