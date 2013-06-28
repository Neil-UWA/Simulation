#ifndef _OPTIMIZE_H
#define _OPTIMIZE_H 1

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "ga.h"
#include "brute.h"

#if !defined(WIDTH) && !defined(HEIGHT)
#define WIDTH	(800)   
#define HEIGHT	(800)  
#endif

#define	COMMENT		'#'
#define	COLOUR_OBJECTS	"#aaaaaa"
#define	COLOUR_PATH	"#80CC80"

#ifndef RADIUS
#define RADIUS (60)
#endif

#define RADIUS2	(RADIUS*RADIUS)

#define FOR_LOOP	for(i=0;i<WIDTH;i++)for(j=0;j<HEIGHT;j++)

#define LOSS 12 //signal loss per building

extern  int	map[WIDTH][HEIGHT];
extern  int	fixed_map[WIDTH][HEIGHT];

typedef enum {
	FREE = 0,
	OBSTACLE,
	PATH,
	COVERED,
	AP
} STATE;

typedef struct _WiFi {
	double	tx_frequency_GHz;

	double	tx_power_dBm;
	double	tx_antenna_gain_dBi;
	double	tx_cable_loss_dBm;

	double	rx_antenna_gain_dBi;
	double	rx_cable_loss_dBm;
	double	rx_sensitivity_dBm;
} WiFi;

extern WiFi wifi;

typedef struct _LOCATION {
	int	x;
	int	y;
	int coverage;
} LOCATION;     

typedef struct _GL {
	int ncolours;
	unsigned char *rs, *gs, *bs;
} GLOBALS;
extern GLOBALS g;
typedef enum {
	T_OBJ	= 0,
	T_TEXT,
	T_POINT,
	T_PATH
} OBJTYPE;

typedef struct {
	OBJTYPE	type;
	char	*str;
	int		x0;
	int		y0;
	int		x1;
	int		y1;
} OBJECT;

typedef struct _INDIVIDUAL {
	LOCATION	locations[NUM_AP];
	double		fitness;			//how many path (pixes) are covered
	double		relative_fitness;	//fitness/total_fitness
} INDIVIDUAL;

extern bool prob(void); 
extern int	through_N_objects(int x0, int y0, int x1, int y1);  
extern bool wlan_model(int x0, int y0, int x1, int y1);
extern void init_map (void);
extern int	get_coverage(LOCATION location);
extern void set_path_covered(LOCATION location);
extern void set_obstacle(int x0, int y0, int x1, int y1);
extern void set_path(int x0, int y0, int x1, int y1);
extern void read_map(const char* file);
extern int	total_path_area(void);
extern int	covered_path(void);
extern double covered_ratio(void);

#endif
