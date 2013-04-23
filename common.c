#include <cnet.h>
#include <string.h>
#include "common.h"


/**
* @brief  initalize a frame
*
* @param kind type of the frame
* @param dst  the target address 
* @param msg Message
*
* @return FRAME 
*/
FRAME	
initFrame(KIND kind, int dst, char	*msg)
{
	FRAME	frame;

	memset(&frame, 0, sizeof(FRAME));

	frame.kind		=	kind;
	frame.dst		=	dst;
	frame.nodeinfo	=	nodeinfo;
	frame.rxsignal	=	0.0;
	strcpy(frame.msg, msg);
	CHECK(CNET_get_position(&(frame.position), NULL));

	return frame;
}

#ifdef DEBUG
void
showFrame(FRAME frame)
{
	printf("from\tkind\tdst\tmsg\t\tsignal strength\n");
	printf("%d\t%d\t%d\t%s\t\t%.3lf\n", 
		frame.nodeinfo.nodenumber, frame.kind, frame.dst, frame.msg, frame.rxsignal);
//	printf("my current location x = %d y = %d\n", frame.position.x, frame.position.y);
//	if (frame.nodeinfo.nodetype == NT_ACCESSPOINT) {
//		printf("signal srenghth %.2lfDB\n",frame.rxsignal);
//	}
}
#endif

/**
* @brief transmit a frame
*
* @param kind
* @param dst
* @param msg
* @param rxsignal
*/
void 
transmit(KIND kind, int dst, char *msg, double rxsignal)
{
	FRAME	frame	=	initFrame(kind, dst, msg);
	size_t	length	=	sizeof(FRAME);
	int		link	=	1;

	frame.rxsignal	=	rxsignal;

	CHECK(CNET_write_physical_reliable(link, (FRAME *)&frame, &length));
	printf("transmitting a %s frame to ap %d\n", msg, dst);
}
