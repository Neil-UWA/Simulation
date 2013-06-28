#include "optimize.h"
#include <float.h>
#include <math.h>

static void init_RGB(int ncolours)
{
	typedef struct _RGB; {
		unsigned char	r;
		unsigned char	g;
		unsigned char	b;
		int		y;
	} RGB;;

	RGB rainbow[] = {
		{ 255, 0, 0, 0 },
		{ 255, 40, 0, 0 },
		{ 255, 80, 0, 0 },
		{ 255, 120, 0, 0 },
		{ 255, 162, 0, 0 },
		{ 255, 255, 0, 0 },
		{ 0, 255, 0, 0 },
		{ 0, 0, 255, 0 },
		{ 111, 0, 255, 0 },
		{ 238, 130, 238, 0 },
		{ 255, 255, 255, 0 },
	};

#define N_RAINBOW (sizeof(rainbow) / sizeof(rainbow[0]))

	if (ncolours < N_RAINBOW) {
		ncolours = N_RAINBOW;	
	}

	g.rs = calloc(ncolours, sizeof(unsigned char));
	g.gs = calloc(ncolours, sizeof(unsigned char));
	g.bs = calloc(ncolours, sizeof(unsigned char));

	//check whether the allocated memory is successful
	if (NULL == g.rs || NULL == g.gs || NULL == g.bs) {
		g.rs = g.gs = g.bs = NULL;
		return;
	}

	if (1 == ncolours) {
		g.rs[0] = rainbow[0].r;
		g.gs[0] = rainbow[0].g;
		g.bs[0] = rainbow[0].b;
		return;
	}

	for (int i = 0; i < N_RAINBOW; i++) {
		rainbow[i].y = ((ncolours-1)*i)/(N_RAINBOW-1);
	}

	RGB *cur = &rainbow[0], *next = &rainbow[i];

	for (int i = 0; i < ncolours; i++) {
		int r0, g0, b0;

		while(next->y < i){
			cur++;
			next++;
		}

		r0 = cur->r + ((next->r - cur->r)*(i-cur->y))/(next->y-cur->y);
		g0 = cur->g + ((next->g - cur->g)*(i-cur->y))/(next->y-cur->y);
		b0 = cur->b + ((next->b - cur->b)*(i-cur->y))/(next->y-cur->y);

		g.rs[i] = r0 & 0xff;
		g.gs[i] = g0 & 0xff;
		g.bs[i] = b0 & 0xff;

	}
#undef N_RAINBOW
}
