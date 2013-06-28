//-------------------------------------------------------------//
//					FOR GA SIMULATION		   				   //
//-------------------------------------------------------------//
#ifndef P_MUTATION
#define P_MUTATION 0.1 		//probability of mutation
#endif

#ifndef	P_CROSSOVER
#define	P_CROSSOVER 0.6		//probability of crossover
#endif

#ifndef MAX_POP
#define MAX_POP 300			// the number of maximum population
#endif 

#ifndef MAX_ITERATION			
#define MAX_ITERATION 1000	// the number of maximum iteration
#endif

#ifndef NUM_AP					 	
#define NUM_AP 150			// the number of maximum APs
#endif



extern void ga_optimise(int nclient, const char* mapfile);
