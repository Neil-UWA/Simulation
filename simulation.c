#include <cnet.h>
#include <time.h>
#include "mapping.h"
#include "walking.h"

EVENT_HANDLER(click){
    time_t  now;
    time(&now);
    printf("hello\n");    
    printf("%s\n", ctime(&now));
}

EVENT_HANDLER(reboot_node){
	char**	argv = (char**) data;
	if(argv[0])
	{
		readmap(argv[0]);
		init_walking();
		start_walking();
		
		CNET_set_handler(EV_DEBUG1, click, 0);
		CNET_set_debug_string(EV_DEBUG1, "click me");
	}
}
