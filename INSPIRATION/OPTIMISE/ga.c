#include "optimize.h"
#include "ga.h"  

// #ifdef USE_GA_OPTIMISE
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

static	INDIVIDUAL the_best;

static void keep_the_best(INDIVIDUAL* population)
{
	memset(&the_best, 0, sizeof(INDIVIDUAL));

	int i;
	FOR_INDIVIDUAL{
		if(the_best.fitness < population[i].fitness)
		{
			memcpy(&the_best, &population[i], sizeof(INDIVIDUAL));
		}
	}
}

static void elitist(INDIVIDUAL* population)
{
	INDIVIDUAL elist;
	INDIVIDUAL worst;
	int	 worst_index;

	elist = population[0];
	worst = population[0];

	for (int i = 0; i < NUM_AP-1; i++) 
	{
		if (population[i].fitness > population[i+1].fitness) {
			if (elist.fitness <= population[i].fitness) 
				memcpy(&elist, &population[i], sizeof(INDIVIDUAL));
					
			if (worst.fitness >= population[i+1].fitness) {
				memcpy(&worst, &population[i+1], sizeof(INDIVIDUAL));
				worst_index = i+1;
			}
		}
		else {
			if (elist.fitness <= population[i+1].fitness) 
				memcpy(&elist, &population[i+1], sizeof(INDIVIDUAL));
					
			if (worst.fitness >= population[i].fitness) {
				memcpy(&worst, &population[i], sizeof(INDIVIDUAL));
				worst_index = i;
			}
		}
	}

	if (the_best.fitness <= elist.fitness) {
		the_best = elist;
	}
	else {
		population[worst_index] = the_best;
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
	fp = fopen("../log1", "a+");

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

	if((fp = fopen("../RESULT", "w"))){
		fprintf(fp, "compile	=	\"simulation.c mapping.c walking.c accesspoint.c client.c common.c -lm\"\n\n");
		fprintf(fp, "rebootargs	= \"%s\"\n\n", mapfile);
		fprintf(fp, "drawlinks	= false\n");
		fprintf(fp, "mapwidth	= %d\n", WIDTH);
		fprintf(fp, "mapheight	= %d\n\n", HEIGHT);

		fprintf(fp, "icontitle	= \"%%a\"\n\n");
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

void ga_optimise(int nclient, const char* mapfile)
{
	int t = 0;
	int index = 0;
	INDIVIDUAL best,temp;

	init_population(population);
	evaluate_fitness(population);

	memset(&best, 0, sizeof(INDIVIDUAL));
	memset(&temp, 0, sizeof(INDIVIDUAL));
	memset(&the_best, 0, sizeof(INDIVIDUAL));

	keep_the_best(population);

	the_best = best_individual(population);

	while(t < MAX_ITERATION)
	{
		selection(population, new_population);
		crossover(new_population);
		mutation(new_population);
		update_population(population, new_population);
		evaluate_fitness(population);
		elitist(population);

		temp = best_individual(population);
		printf("Generation [%d]: best combination with coverage %lf \n", t, temp.fitness);
		for(int i = 0; i < NUM_AP; ++i)
		{
			printf("\t%d %d\n", temp.locations[i].x, temp.locations[i].y);
		}
		if (best.fitness < temp.fitness) {
			best = temp;
			index = t;
		}

		t++;
	}

	printf("Best combination occured in generation [%d] with coverage: %lf\n", index, best.fitness);
	for(int i = 0; i < NUM_AP; ++i)
	{
		printf("%d %d\n", best.locations[i].x, best.locations[i].y);
	}                                                              
	
	output(best, nclient, mapfile);

}
// #endif

//-------------------------------------------------------------//
