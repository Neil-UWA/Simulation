#include "optimize.h"

static	OBJECT	*objects	= NULL;
static	int	nobjects	= 0;
//static	OBJECT	**paths		= NULL;
static	int	npaths		= 0; 

int	map[WIDTH][HEIGHT];
int	fixed_map[WIDTH][HEIGHT];


bool prob(void)
{
	return rand()< RAND_MAX*0.5;
}

void init_map (void)
{
	int i, j;
	FOR_LOOP{
		map[i][j] = FREE;
		fixed_map[i][j] = FREE;
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
	int		n = 0;

	TXtotal = wifi.tx_power_dBm - wifi.tx_cable_loss_dBm + wifi.tx_antenna_gain_dBi;

	metres = sqrt(distance(x0, y0, x1, y1));

	FSL = (92.467 + 20.0*log10(wifi.tx_frequency_GHz) + 20.0*log10(metres/1000.0));
	n = through_N_objects(x0, y0, x1, y1);
	rx_strength_dBm = TXtotal - FSL + wifi.rx_antenna_gain_dBi - wifi.rx_cable_loss_dBm - n*LOSS;

	budget = rx_strength_dBm - wifi.rx_sensitivity_dBm;

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
int get_coverage(LOCATION location)
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
		//IS THE PATH IN THE COVERAGE OF THE AP?
		if (map[i][j] == PATH){
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
void set_path_covered(LOCATION location)
{
	int i, j;
	int	min_x, min_y;
	int	max_x, max_y;

	min_x = location.x - RADIUS >0 ? location.x - RADIUS : 0;
	min_y = location.y - RADIUS >0 ? location.y - RADIUS : 0;
	max_x = location.x + RADIUS > WIDTH ? WIDTH : location.x + RADIUS;
	max_y = location.y + RADIUS > HEIGHT ? HEIGHT : location.y + RADIUS;

	for (i = min_x; i <= max_x; i++) for (j = min_y; j <= max_y; j++) {
		//IS THE PATH IN THE COVERAGE OF THE AP?
		if (map[i][j] == PATH){
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
 * @brief add OBSTACLEs into the map
 *
 * @param x0
 * @param y0HEIGHT
 * @param x1
 * @param y1
 */
void set_obstacle(int x0, int y0, int x1, int y1)
{
	int	dx, dy;
	int temp;

	dx = abs(x1 - x0);
	dy = abs(y1 - y0);

	if (x0>x1) temp = x0, x0 = x1, x1 = temp;
	if (y0>y1) temp = y0, y0 = y1, y1 = temp;	

	for (int i = 0 ; i <= dx; i++) {
		for (int j = 0 ; j <= dy; j++) {
			map[x0+i][y0+j] = OBSTACLE;
			fixed_map[x0+i][y0+j] = OBSTACLE;
		}
	}
}

void set_path(int x0, int y0, int x1, int y1)
{
	int	dx, dy;
	int temp;

	dx = abs(x1 - x0);
	dy = abs(y1 - y0);

	if (x0>x1) temp = x0, x0 = x1, x1 = temp;
	if (y0>y1) temp = y0, y0 = y1, y1 = temp;	

	for (int i = 0 ; i <= dx; i++) {
		for (int j = 0 ; j <= dy; j++) {
			map[x0+i][y0+j] = PATH;
			fixed_map[x0+i][y0+j] = PATH;
		}
	}
}

//get the area of paths
int total_path_area(void)
{
	int i, j;
	int total = 0;
	FOR_LOOP{
		if (fixed_map[i][j] == PATH) 
			total += 1;			
	}
	return total;
}

//get how many paths are covered
int covered_path(void)
{
	int i, j;
	int covered = 0;
	FOR_LOOP{
		if (map[i][j] == COVERED) 
			covered +=1;
	}

	return covered;
}

double covered_ratio(void)
{
	return (double)covered_path()/(double)total_path_area();
}

#define	FOREACH_OBJECT	for(n=0, op=objects ; n<nobjects ; ++n, ++op)

static void add_object(OBJTYPE type, const char *str,
		int x0, int y0, int x1, int y1)
{
	objects	= realloc(objects, (nobjects+1)*sizeof(OBJECT));

	if(objects) {

		OBJECT	*new	= &objects[nobjects];

		memset(new, 0, sizeof(OBJECT));
		new->type	= type;
		if(str)
			new->str	= strdup(str);

		if(type == T_PATH) {
			if(x0 != x1 && y0 != y1)
				return;
		}

		//  ENSURE THAT THE FIRST COORD IS 'LESS THAN' THE SECOND
		if(type == T_OBJ || type == T_PATH) {
			double	t;

			if(x0 > x1)
				t = x1, x1 = x0, x0 = t;
			if(y0 > y1)
				t = y1, y1 = y0, y0 = t;
		}
		new->x0		= x0;
		new->y0		= y0;
		new->x1		= x1;
		new->y1		= y1;
		++nobjects;

		if(type == T_PATH)
			++npaths;
	}
	else
		nobjects = 0;
}

static bool find_point(const char *str, int *x, int *y)
{
	int		n;
	OBJECT	*op;

	FOREACH_OBJECT {
		if(op->type == T_POINT && strcmp(op->str, str) == 0) {
			*x	= op->x0;
			*y	= op->y0;
			return true;
		}
	}
	return false;
}

static char *trim(char *line)
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

static bool intersect(	int x0, int y0, int x1, int y1,
		int x2, int y2, int x3, int y3)
{
#define	EPS	0.0001
	double	denom  = (y3-y2) * (x1-x0) - (x3-x2) * (y1-y0);
	double	numera = (x3-x2) * (y0-y2) - (y3-y2) * (x0-x2);
	double	numerb = (x1-x0) * (y0-y2) - (y1-y0) * (x0-x2);

	// ARE LINES COINCIDENT?
	if(fabs(numera) < EPS && fabs(numerb) < EPS && fabs(denom) < EPS)
		return true;

	// ARE LINES PARALLEL?
	if(fabs(denom) < EPS)
		return false;

	// IS INTERSECTION ALONG THE THE SEGMENTS?
	double	mua = numera / denom;
	double	mub = numerb / denom;
	return (mua > 0 && mua < 1 && mub > 0 && mub < 1);
#undef	EPS
}

int through_N_objects(int x0, int y0, int x1, int y1)
{
	int		n;
	OBJECT	*op;
	int	count	= 0;

	FOREACH_OBJECT {
		if(op->type == T_OBJ)		// only interested in objects
			if(	intersect(x0, y0, x1, y1, op->x0, op->y0, op->x1, op->y0) ||
					intersect(x0, y0, x1, y1, op->x1, op->y0, op->x1, op->y1) ||
					intersect(x0, y0, x1, y1, op->x1, op->y1, op->x0, op->y1) ||
					intersect(x0, y0, x1, y1, op->x0, op->y1, op->x0, op->y0) )
				++count;
	}
	return count;
}

void read_map(const char* file)
{
	FILE *fp = NULL;

	fp=fopen(file, "r");    

	//  EACH NODE READS IN THE MAP DETAILS
	if(fp) {
		char	line[1024], str0[1024], str1[1024], *s;

		while(fgets(line, sizeof(line), fp)) {
			s	= trim(line);

			if(*s) {
				int	x0, y0, x1, y1;

				if(sscanf(s, "object %d %d %d %d", &x0, &y0, &x1, &y1) == 4){
					add_object(T_OBJ, NULL, x0, y0, x1, y1);
					set_obstacle(x0,y0,x1,y1);
				}
				else if(sscanf(s, "text %d %d %s", &x0, &y0, str0) == 3)
					add_object(T_TEXT, str0, x0, y0, 0, 0);
				else if(sscanf(s, "point %d %d %s", &x0, &y0, str0) == 3)
					add_object(T_POINT, str0, x0, y0, 0, 0);
				else if(sscanf(s, "path %d %d %d %d", &x0, &y0, &x1, &y1) == 4){
					add_object(T_PATH, NULL, x0, y0, x1, y1);
					set_path(x0, y0, x1, y1);
				}
				else if(sscanf(s, "path %s %s", str0, str1) == 2) {
					if(find_point(str0, &x0, &y0) && find_point(str1, &x1, &y1)){
						add_object(T_PATH, NULL, x0, y0, x1, y1);
						set_path(x0, y0, x1, y1);
					}
				}
			}
		}
		fclose(fp);
	}
}

void usage(void)
{
#ifdef USE_GA_OPTIMISE
	fprintf(stderr, "please input the number of clients, and the map\n");
	fprintf(stderr, "./optimize nclient mapfile\n");
#else
	fprintf(stderr, "please input the number of APs and clients, and the map\n");
	fprintf(stderr, "./optimize nap nclient mapfile\n");

#endif
}

#ifdef USE_BRUTE_SEARCH
int main(int argc, const char *argv[])
{
	int nap;
	int nclient;
	char *mapfile;

	if (argc != 4) 
		usage();
	else {
		nap = atoi(argv[1]);
		nclient = atoi(argv[2]);
		mapfile = strdup(argv[3]);

		srand(time(NULL));

		init_map();	
		read_map(mapfile);

		brute_optimise(nap, nclient, mapfile);
	}
	return 0;
}
#endif

#ifdef USE_GA_OPTIMISE
int main (int argc, char const *argv[])
{
	int nclient;
	char *mapfile;

	if (argc != 3) 
		usage();
	else {
		nclient = atoi(argv[1]);
		mapfile = strdup(argv[2]);

		srand(time(NULL));

		init_map();	
		read_map(mapfile);

		ga_optimise(nclient, mapfile);
	}

	return 0;
}
#endif
