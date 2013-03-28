#include <cnet.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "coverage.h"
#include "am.h"


EVENT_HANDLER(monitor){
	cell	cellFrame;
	size_t	cell_len= sizeof(cell);

	FRAME	frame;
	size_t  frame_len= sizeof(FRAME);

	void	*object = NULL;
	size_t	len = cell_len + frame_len;
	int		link;

	object = (void *) malloc(len);

	CHECK(CNET_read_physical(&link, object, &len));

	if (len == cell_len) {
		memcpy(&cellFrame, (cell*)object, cell_len);
		printf("received x: %d, y: %d, state: %d\n", cellFrame.current.x, cellFrame.current.y, cellFrame.cellState);
		set_covered(cellFrame.current);
	}
	else {
		memcpy(&frame, (FRAME*)object, frame_len);
	}

	printf("%.2lf\n",coverage_rate());
}
