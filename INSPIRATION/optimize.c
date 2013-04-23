#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "optimize.h"

#define LOSS 12

static int	map[WIDTH][HEIGHT];
static int	fixed_map[WIDTH][HEIGHT];

static	OBJECT	*objects	= NULL;
static	int	nobjects	= 0;
static	OBJECT	**paths		= NULL;
static	int	npaths		= 0; 

extern char *strdup(const char *str);
static int	through_N_objects(int x0, int y0, int x1, int y1);

static bool prob()
{
	return rand()< RAND_MAX*0.5;
}

static void init_map ()
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

	TXtotal = tx_power_dBm - tx_cable_loss_dBm + tx_antenna_gain_dBi;

	metres = sqrt(distance(x0, y0, x1, y1));

	FSL = (92.467 + 20.0*log10(tx_frequency_GHz) + 20.0*log10(metres/1000.0));
	n = through_N_objects(x0, y0, x1, y1);
	rx_strength_dBm = TXtotal - FSL + rx_antenna_gain_dBi - rx_cable_loss_dBm - n*LOSS;

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
static void set_path_covered(LOCATION location)
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
static void set_obstacle(int x0, int y0, int x1, int y1)
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

static void set_path(int x0, int y0, int x1, int y1)
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

static void read_map(const char* file)
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

//get the area of paths
static int total_path_area()
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
static int covered_path()
{
	int i, j;
	int covered = 0;
	FOR_LOOP{
		if (map[i][j] == COVERED) 
			covered +=1;
	}

	return covered;
}

static double covered_ratio()
{
	return (double)covered_path()/(double)total_path_area();
}

#ifdef USE_BRUTE_SEARCH
//-------------------------------------------------------------//
//			BRUTE SEARCH									   //
//-------------------------------------------------------------//
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
		if (map[i][j] != AP) {
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
//	set_covered(best);
	set_path_covered(best);

	return best;
}

/**
 * @brief start simulation and produce a topology file
 *
 * @param nap number of APs
 */
//void brute_optimise(int nap, int nclient, const char *file)
void brute_optimise(double nap, int nclient, const char *file)
{
	FILE *fp = NULL;
	int index	= 0;
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
		
		printf("cover rate :\n", covered_ratio());
		while(covered_ratio()<nap){
			best = brute_search();
			printf("somehting\n");

#ifdef	DEBUG
			printf("%d %d coverage:%d\n", best.x, best.y,best.coverage);
#endif
			
			fprintf(fp, "accesspoint AP%d {\n\tx = %d, y = %d \n\twlan {}\n}\n\n", index, best.x, best.y);
			fflush(fp);

			memset(&best, 0, sizeof(LOCATION));
			index ++;
		}
		printf("the final coverage rate is %lf\n", covered_ratio());

		for (int i = 0; i < nclient; i++) {
			fprintf(fp, "mobile m%d{wlan{}}\n", i);
		}
	}

	fflush(fp);
	fclose(fp);
}
#endif

#ifdef USE_GA_OPTIMISE
//-------------------------------------------------------------//
//	GA simulation starts here								   //
//-------------------------------------------------------------// 

#define FOR_INDIVIDUAL	for(i=0;i<MAX_POP;i++)

static INDIVIDUAL population[MAX_POP];
static INDIVIDUAL new_population[MAX_POP];
static double	  total_fitness = 0.0;

static LOCATION rand_location()
{
	LOCATION location;

	location.x = rand()%WIDTH;
	location.y = rand()%HEIGHT;
	location.coverage = 0;

	return location;
}

static bool same_location(LOCATION src, LOCATION tg)
{
	return memcmp(&src, &tg, sizeof(LOCATION));
}

static void refresh_map()
{	
	int i, j;
	FOR_LOOP{
		memcpy(&map[i][j],&fixed_map[i][j], sizeof(WIDTH*HEIGHT)); 
	}
}

static void init_population(INDIVIDUAL* population)
{	
	int i;
	FOR_INDIVIDUAL {
		for (int j = 0; j < NUM_AP; j++) {

			int k = 0;

			//generate a location, which is not equal to any of the other locations
			if (j == 0) population[i].locations[j] = rand_location();
			else 
			{
				while (k<j) 
				{
					population[i].locations[j] = rand_location();
					if (!same_location(population[i].locations[j], population[i].locations[k])) {
						population[i].locations[j] = rand_location();
						k=0;
					}
					else k++;
				}		
			}

		}
	}
}

static double get_relative_fitness(INDIVIDUAL individual)
{
	return (individual.fitness/total_fitness);
}

static void evaluate_fitness(INDIVIDUAL* population)
{
	int i;

	FOR_INDIVIDUAL{
		for(int j = 0; j < NUM_AP; ++j)
			set_path_covered(population[i].locations[j]);

		population[i].fitness = (double) covered_path();
		total_fitness += population[i].fitness;

		refresh_map();
	}

	FOR_INDIVIDUAL{
		population[i].relative_fitness = get_relative_fitness(population[i]);
	}
}

//roulette wheel selection
static void selection(INDIVIDUAL* population, INDIVIDUAL* new_population)
{	
	double accumulate_fitness = 0.0;
	double select_probability = 0.0;

	int i;

	FOR_INDIVIDUAL{
		select_probability = (rand()%100)/100.0;
		accumulate_fitness = 0.0;

		for(int j = 0; j < MAX_POP; ++j)
		{
			accumulate_fitness += population[j].relative_fitness;

			if(accumulate_fitness>select_probability)
			{
				memcpy(&new_population[i],&population[j], sizeof(INDIVIDUAL));
				break;
			}
		}
	}	
}

static void random_swap(INDIVIDUAL* father, INDIVIDUAL* mother)
{
	LOCATION* temp = NULL;
	int position = 0;
	int	num_swaped_location = 0;

	position = rand()%(NUM_AP-1);
	num_swaped_location = NUM_AP - position - 1;
	temp = realloc(temp,num_swaped_location*sizeof(LOCATION));

	for (int i = 0; i < num_swaped_location; i++) {
		temp[i] = father->locations[position+1+i];
		father->locations[position+1+i] = mother->locations[position+1+i];
		mother->locations[position+1+i] = temp[i];
	}

	free(temp);
	temp = NULL;
}

//one-point crossover
static void crossover(INDIVIDUAL* population)
{
	double cross_probability = 0.0;
	int		position = 0;
	int		temp;
	int		index[MAX_POP];

	// make pairs randomly
	for (int i = 0; i < MAX_POP; i++) 
		index[i] = i;

	for (int i = 0; i < MAX_POP; i++) {
		position = rand()%(MAX_POP-i);
		temp = index[i];
		index[i] = index[position+i];
		index[position+i] = temp;
	}

	for(int i = 0; i < MAX_POP/2; ++i)
	{
		cross_probability = (rand()%100)/100.0;

		if(cross_probability <= P_CROSSOVER)
			random_swap(&population[index[i]], &population[index[i+1]]);
	}
}

static void mutation(INDIVIDUAL* population)
{
	int		mutation_point;                 //which location mutates
	double	mutation_probability = 0.0;
	double  whole_probability = 0.0;       	//probability of the whole location change
	double 	part_probability = 0.0;         //probability of x coordinator, or y coordinator changes
	LOCATION temp;                                                                                 


	int i;
	FOR_INDIVIDUAL{
		mutation_probability = (rand()%100)/100.0;

		if(mutation_probability <= P_MUTATION)
		{	
			mutation_point = rand()%(NUM_AP);
			whole_probability = (rand()%100)/100.0;
			temp = rand_location();

			if(whole_probability < 0.5)
				population[i].locations[mutation_point] = temp;
			else
			{
				part_probability = (rand()%100)/100.0;
				if(part_probability<0.5)
					population[i].locations[mutation_point].x = temp.x;
				else
					population[i].locations[mutation_point].y = temp.y;
			}
		}
	}		
}

static void update_population(INDIVIDUAL* old, INDIVIDUAL* new)
{
	int i;

	FOR_INDIVIDUAL{
		memcpy(&old[i],&new[i],sizeof(INDIVIDUAL));
	}
}

static int	gen = 0;
//get the best combination of locations of the current population
static INDIVIDUAL best_individual(INDIVIDUAL* population)
{
	INDIVIDUAL best;
	int i;           

	FILE *fp = NULL;
	fp = fopen("log", "a+");

	memset(&best, 0, sizeof(INDIVIDUAL));
	FOR_INDIVIDUAL{
		if(best.fitness < population[i].fitness)
		{
			memcpy(&best, &population[i], sizeof(INDIVIDUAL));
		}
	}

	fprintf(fp, "%d %.1lf\n", gen, best.fitness);
	gen ++;
	return best;
}


//output the best combination of AP locations
static void output(INDIVIDUAL individual, int nclient, 
		const char* mapfile)
{
	FILE	*fp = NULL;
	int i;

	if((fp = fopen("RESULT", "w"))){
		fprintf(fp, "compile	=	\"simulation.c mapping.c walking.c accesspoint.c client.c common.c -lm\"\n\n");
		fprintf(fp, "rebootargs	= \"%s\"\n\n", mapfile);
		fprintf(fp, "drawlinks	= false\n");
		fprintf(fp, "mapwidth	= %d\n", WIDTH);
		fprintf(fp, "mapheight	= %d\n\n", HEIGHT);

		fprintf(fp, "icontitle	= \"\%\%a\"\n\n");
		fprintf(fp, "mapscale	= 0.3\n");
		fprintf(fp, "mapgrid	= 10\n\n");


		for(int j = 0; j < NUM_AP; ++j)
		{
			fprintf(fp, "accesspoint AP%d {\n\tx = %d, y = %d \n\twlan {}\n}\n\n", j, individual.locations[j].x, individual.locations[j].y);
			fflush(fp);
		}

		for(int i = 0; i < nclient; ++i)
			fprintf(fp, "mobile m%d{wlan{}}\n", i);


		fflush(fp);
		fclose(fp);
	}
}

static void ga_optimise(int nclient, const char* mapfile)
{
	int t = 0;
	int index = 0;
	INDIVIDUAL best,temp;

	init_population(population);
	evaluate_fitness(population);

	memset(&best, 0, sizeof(INDIVIDUAL));
	memset(&temp, 0, sizeof(INDIVIDUAL));

	while(t < MAX_ITERATION)
	{
		selection(population, new_population);
		crossover(new_population);
		mutation(new_population);
		update_population(population, new_population);
		evaluate_fitness(population);

		temp = best_individual(population);
		printf("Generation [%d]: best combination with coverage %lf\n", t, temp.fitness);

		if (best.fitness < temp.fitness) {
			best = temp;
			index = t;
		}

		t++;
	}

	printf("Best combination occured in generation [%d] with coverage: %lf\n", index, best.fitness);
	output(best, nclient, mapfile);

}
#endif

//-------------------------------------------------------------//

#ifdef DEBUG
static void show_population(INDIVIDUAL* population)
{
	for (int i = 0; i < MAX_POP; i++) {
		for (int j = 0; j < NUM_AP; j++) {
			printf(" (%d,%d) ",population[i].locations[j].x, population[i].locations[j].y);
		}
		printf("coverage: %lf, rfitness: %lf\n", population[i].fitness, population[i].relative_fitness);
	}
}

static void show_map()
{
	for (int i = 0; i < WIDTH; i++) {
		for (int j = 0; j < HEIGHT; j++) {
			if (map[i][j] == PATH ) {
			printf("%d", map[i][j]);
			}
		}
		printf("\n");
	}
}
#endif

int main (int argc, char const *argv[])
{
	double nap, nclient;
	char *mapfile;

	if (argc != 4) {
		fprintf(stderr, "please input the number of APs and clients, and the map\n");
		fprintf(stderr, "./optimize nap nnode mapfile\n");
	}
	else {
		nap = atof(argv[1]);
		nclient = atoi(argv[2]);
		mapfile = strdup(argv[3]);

		srand(time(NULL));

		init_map();	
		read_map(mapfile);

#ifdef USE_GA_OPTIMISE
		ga_optimise(nclient, mapfile);
#endif

#ifdef USE_BRUTE_SEARCH
		brute_optimise(nap, nclient, mapfile);
#endif

	}

	return 0;
}


