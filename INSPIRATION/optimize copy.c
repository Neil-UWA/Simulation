#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#ifndef WIDTH && HEIGHT 
#define WIDTH	(600)   // MAP WIDTH 
#define HEIGHT	(1200)  // MAP HEIGHT
#endif

#define	COMMENT		'#'
#define	COLOUR_OBJECTS	"#aaaaaa"
#define	COLOUR_PATH	"#80CC80"

// defualt values for transmitter and receiver
#define tx_frequency_GHz (2.45)	
#define tx_power_dBm (14.771)		
#define	tx_antenna_gain_dBi (2.14)
#define tx_cable_loss_dBm (0.0)	
#define rx_antenna_gain_dBi (1.14)
#define rx_cable_loss_dBm (0.0)	
#define rx_sensitivity_dBm (-82.0)  

#ifndef RADIUS
#define RADIUS (60)
#endif
#define RADIUS2	(RADIUS*RADIUS)

#define FOR_LOOP	for(i=0;i<WIDTH;i++)for(j=0;j<HEIGHT;j++)

typedef enum {
	FREE = 0,
	OBSTACLE,
	COVERED,
	AP
} STATE;

typedef struct _LOCATION {
	int	x;
	int	y;
	int coverage;
} LOCATION;

static int	map[WIDTH][HEIGHT];
static int	outdoor = 0;
static int  covered = 0;

static bool prob()
{
	return rand() < RAND_MAX*0.5;
}

static void init_map ()
{
	int i, j;
	FOR_LOOP{
		map[i][j] = FREE;
	} 
}

/**
 * @brief distance between 2 points
 *
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 *
 * @return 
 */
static double distance(int x0, int y0, int x1, int y1)
{
	return ((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
}

#ifdef USE_WLAN_MODEL
/**
* @brief free space path loss model
*
* @param x0 receiver's x coordinator
* @param y0 receiver's y coordinator
* @param x1 AP's x coordinator
* @param y1 AP's y coordinator
*
* @return 
*/
static bool wlan_model(int x0, int y0, int x1, int y1)
{
	double TXtotal, FSL, budget, rx_strength_dBm;
	double metres;

	TXtotal = tx_power_dBm - tx_cable_loss_dBm + tx_antenna_gain_dBi;

	metres = sqrt(distance(x0, y0, x1, y1));

	FSL = (92.467 + 20.0*log10(tx_frequency_GHz) + 20.0*log10(metres/1000.0));

	rx_strength_dBm = TXtotal - FSL + rx_antenna_gain_dBi - rx_cable_loss_dBm;

	budget = rx_strength_dBm - rx_sensitivity_dBm;

	if (budget < 0.0) 
		return false;

	return true;
}
#endif

/**
 * @brief get the coverage of an AP that is not covered by other APs
 *
 * @param x x-coordinator
 * @param y y-coordinator
 *
 * @return 
 */
static int get_coverage(LOCATION location)
{
	int i, j;
	int	min_x, min_y;
	int	max_x, max_y;
	int total_coverage = 0;

	min_x = location.x - RADIUS >0 ? location.x - RADIUS : 0;
	min_y = location.y - RADIUS >0 ? location.y - RADIUS : 0;
	max_x = location.x + RADIUS > WIDTH ? WIDTH : location.x + RADIUS;
	max_y = location.y + RADIUS > HEIGHT ? HEIGHT : location.y + RADIUS;

	for (i = min_x; i <= max_x; i++) for (j = min_y; j <= max_y; j++) {
		//IS THE POINT IS IN THE COVERAGE OF THE AP?
		if (map[i][j] == FREE){
#ifndef USE_WLAN_MODEL
			if (distance(i, j, location.x, location.y) <= RADIUS2) 
				total_coverage += 1;
#else
			if (wlan_model(i, j, location.x, location.y)) 
				total_coverage += 1;
#endif
		}
	}

	return total_coverage;
}

/**
 * @brief when a AP is located, set the state of all points 
 * covered by this AP to covered
 *
 * @param location AP's location
 */
static void set_covered(LOCATION location){
	int i, j;
	int	min_x, min_y;
	int	max_x, max_y;

	min_x = location.x - RADIUS >0 ? location.x - RADIUS : 0;
	min_y = location.y - RADIUS >0 ? location.y - RADIUS : 0;
	max_x = location.x + RADIUS > WIDTH ? WIDTH : location.x + RADIUS;
	max_y = location.y + RADIUS > HEIGHT ? HEIGHT : location.y + RADIUS;

	for (i = min_x; i <= max_x; i++) for (j = min_y; j <= max_y; j++) {
		//IS THE POINT IS IN THE COVERAGE OF THE AP?
		if (map[i][j] == FREE){
#ifndef USE_WLAN_MODEL
			if (distance(i, j, location.x, location.y) <= RADIUS2) 
				map[i][j] = COVERED;
#else
			if (wlan_model(i, j, location.x, location.y)) 
				map[i][j] = COVERED;
#endif
		}
	}
}

/**
 * @brief search for the best position for an AP
 *
 * @return the best location
 */
static LOCATION brute_search()
{
	LOCATION best, temp;
	int i, j;

	memset(&best, 0, sizeof(LOCATION));
	memset(&temp, 0, sizeof(LOCATION));

	FOR_LOOP{
		//INSDE A BUILDING 
		if (map[i][j] == OBSTACLE) {
			temp.x = i;
			temp.y = j;
			temp.coverage = get_coverage(temp);
			//REMEMBER THE LOCATION WITH BEST COVERAGE
			if (temp.coverage > best.coverage) 
				best = temp;

			if (temp.coverage == best.coverage) 
				if (prob()) 
					best = temp;
		}
	}

	//ONCE WE FIND THE BEST LOCATION, SET AP THERE
	map[best.x][best.y] = AP;
	set_covered(best);

	return best;
}

/**
 * @brief add OBSTACLEs into the map
 *
 * @param x0
 * @param y0HEIGHT
 * @param x1
 * @param y1
 */
static void set_obstacle(int x0, int y0, int x1, int y1)
{
	int	dx, dy;

	dx = x1 - x0;
	dy = y1 - y0;

	for (int i = 0 ; i <= dx; i++) {
		for (int j = 0 ; j <= dy; j++) {
			map[x0+i][y0+j] = OBSTACLE;
		}
	}
}

char *trim(char *line)
{
	char	*s = line;

	while(*s) {
		if(*s == COMMENT || *s == '\n' || *s == '\r') {
			*s	= '\0';
			break;
		}
		++s;
	}
	s	= line;
	while(isspace(*s))
		++s;
	return s;
}

static void read_map(const char* file)
{
	FILE *fp;
	int x0, y0, x1, y1;

	if ((fp=fopen(file, "r"))) {
		char line[1024], *s;

		while(fgets(line, sizeof(line),fp)){
			s = trim(line);

			if (*s) 
				if(sscanf(s, "object %d %d %d %d", &x0, &y0, &x1, &y1) == 4)
					set_obstacle(x0,y0,x1,y1);
		}
	}

	fclose(fp);
}

/**
 * @brief get the outdoor area
 *
 * @return 
 */
static void out_area()
{
	int i, j;

	FOR_LOOP{
		if (map[i][j] == FREE) 
			outdoor += 1;			
	}
}

/**
 * @brief get the covered_area
 *
 * @return 
 */
static void covered_area()
{
	int i, j;
	FOR_LOOP{
		if (map[i][j] == COVERED) 
			covered +=1;
	}
}

static double ratio()
{
	return (double)covered/(double)outdoor;
}

/**
 * @brief start simulation and produce a topology file
 *
 * @param nap number of APs
 */
void start_simulation(int nap, int nclient, const char *file)
{
	FILE *fp = NULL;
	LOCATION best;

	if((fp = fopen("RESULT", "w"))){
		fprintf(fp, "compile	=	\"simulation.c mapping.c walking.c accesspoint.c client.c common.c -lm\"\n\n");
		fprintf(fp, "rebootargs	= \"%s\"\n\n", file);
		fprintf(fp, "drawlinks	= false\n");
		fprintf(fp, "mapwidth	= %d\n", WIDTH);
		fprintf(fp, "mapheight	= %d\n\n", HEIGHT);

		fprintf(fp, "icontitle	= \"\%\%a\"\n\n");
		fprintf(fp, "mapscale	= 0.3\n");
		fprintf(fp, "mapgrid	= 10\n\n");
		
		while(nap > 0){
			best = brute_search();

#ifdef	DEBUG
			printf("%d %d coverage:%d\n", best.x, best.y,best.coverage);
#endif
			
			fprintf(fp, "accesspoint AP%d {\n\tx = %d, y = %d \n\twlan {}\n}\n\n", nap, best.x, best.y);
			fflush(fp);

			memset(&best, 0, sizeof(LOCATION));
			nap -=1;	
		}
	}

	fflush(fp);
	fclose(fp);
}

int main (int argc, char const *argv[])
{
	int nap;
	int nclient;
	char *map_file;

	if (argc != 4) {
		fprintf(stderr, "please input the number of APs and clients, and the map\n");
		fprintf(stderr, "./optimize nap nnode map\n");
	} 
	else {

		nap = atoi(argv[1]);
		nclient = atoi(argv[2]);
		map_file = argv[3];

		srand(time(NULL));

		init_map();
		read_map(map_file);
		start_simulation(nap, nclient, map_file);
	}

	return 0;
}
