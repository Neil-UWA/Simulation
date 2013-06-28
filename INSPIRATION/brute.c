#include "optimize.h"
#include "brute.h"   

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
	set_path_covered(best);

	return best;
}

/**
 * @brief start simulation and produce a topology file
 *
 */
void brute_optimise(int nap, int nclient, const char *file)
//void brute_optimise(double rate, int nclient, const char *file)
{
	clock_t start, end;
	FILE *fp = NULL;
	int index	= 0;
	LOCATION best;

	if((fp = fopen("RESULT", "w"))){
		fprintf(fp, "compile	=	\"simulation.c mapping.c walking.c accesspoint.c client.c common.c -lm\"\n\n");
		fprintf(fp, "rebootargs	= \"%s\"\n\n", file);
		fprintf(fp, "drawlinks	= false\n");
		fprintf(fp, "mapwidth	= %d\n", WIDTH);
		fprintf(fp, "mapheight	= %d\n\n", HEIGHT);

		fprintf(fp, "icontitle	= \"\%%a\"\n\n");
		fprintf(fp, "mapscale	= 0.6\n");
		fprintf(fp, "mapgrid	= 10\n\n");
		
		start = clock();
//		while(covered_ratio() < rate){
		while(index < nap){
			best = brute_search();
			
			printf("AP %d: (%d, %d)\n", index, best.x, best.y);
			fprintf(fp, "accesspoint AP%d {\n\tx = %d, y = %d \n\twlan {}\n}\n\n", index, best.x, best.y);
			fflush(fp);

			memset(&best, 0, sizeof(LOCATION));
			index ++;
		}

		printf("the final coverage rate is %.2lf\n", covered_ratio());
		printf("Number of APs needs : %d\n", index);

		for (int i = 0; i < nclient; i++) 
			fprintf(fp, "mobile m%d{wlan{}}\n", i);
	}
	end = clock();
	
	printf("Time used: %.2f secs\n", ((double)end - start)/CLOCKS_PER_SEC);
	fflush(fp);
	fclose(fp);
}
