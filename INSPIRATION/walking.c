/** 
 * @file walking.c
 * @brief functions that realted to walking
 * @author Chris McDonald modified by JINANG LU
 */
#include <cnet.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "mapping.h"
#include "walking.h"

#if	!defined(WALK_ON_PATHS) && !defined(WALK_OUTSIDE_RANDOMLY)
#define	WALK_ON_PATHS	1
#endif


#define	WALK_FREQUENCY	1000000		// take a step every 1 second
#define	MAX_SPEED	5		// metres per WALK_FREQUENCY
#define	MIN_PAUSE	5		// in seconds
#define	MAX_PAUSE	30		// in seconds
#define	MAX_WALK_DIST	100		// maximum walk from current point 

static	CnetTimerID	tid		= NULLTIMER;
static	bool		paused		= true;

static EVENT_HANDLER(walker)
{
	static	double	dx	= 0.0;
	static	double	dy	= 0.0;
	static	double	newx	= 0.0;
	static	double	newy	= 0.0;
	static	int	nsteps	= 0;

	CnetPosition	now;
	CnetTime	movenext;

	//  IF PAUSED, WE NEED TO CHOOSE A NEW DESTINATION AND WALKING SPEED
	if(paused) {
		CnetPosition	newdest;

		CHECK(CNET_get_position(&now, NULL));

		//  CHOOSE A NEW DESTINATION THAT DOESN'T REQUIRE WALKING THROUGH A WALL!
		do {
			int	newspeed;
			double	dist;

			newdest	= now;
#if	defined(WALK_ON_PATHS)
			choose_path_position(&newdest, now);
#else
			do {
				choose_outside_position(&newdest, MAX_WALK_DIST);
			} while(through_an_object(now, newdest));
#endif

			dx		= newdest.x - now.x;
			dy		= newdest.y - now.y;
			dist		= sqrt(dx*dx + dy*dy);	// only walking in 2D

			newspeed	= CNET_rand() % MAX_SPEED + 1;
			nsteps	= dist / newspeed;
		} while(nsteps < 3);		// ensure we'll take at least 3 steps

		//  CALCULATE MOVEMENTS PER STEP
		dx	= (dx / nsteps);
		dy	= (dy / nsteps);
		newx	= now.x;
		newy	= now.y;
		paused	= false;		// and off we go....
	}

	//  WE'RE WALKING;  DO WE STILL HAVE SOME STEPS TO TAKE?
	if(nsteps > 0) {
		newx	+= dx;
		newy	+= dy;

		now.x	 = newx;
		now.y	 = newy;
		now.z	 = 0;
		CHECK(CNET_set_position(now));
		DRAW_node_movement(now.x, now.y);	// ADDED FOR Neil's SIMULATION
		paused	= false;
		--nsteps;
		movenext = WALK_FREQUENCY;
	}
	//  WE'VE FINISHED WALKING, SO WE PAUSE HERE FOR A WHILE
	else {
		paused	= true;
		nsteps	= 0;
		movenext = (CNET_rand() % (MAX_PAUSE-MIN_PAUSE) + MIN_PAUSE) * 1000000;
	}

	//  RESCHEDULE THIS WALKING EVENT
	tid	= CNET_start_timer(EV_WALKING, movenext, data);
}

// -----------------------------------------------------------------------

void init_walking(void)
{
	CnetPosition	bigbang = { 0, 0, 0 };

	CHECK(CNET_set_handler(EV_WALKING, walker, 0));

#if	defined(WALK_ON_PATHS)
	choose_path_position(&bigbang, bigbang);
#else
	choose_outside_position(&bigbang, 0);
#endif
	CHECK(CNET_set_position(bigbang));
}

void start_walking(void)
{
	if(tid != NULLTIMER)
	    CNET_stop_timer(tid);
	tid	= CNET_start_timer(EV_WALKING, WALK_FREQUENCY, 0);
}

void stop_walking(void)
{
	if(tid != NULLTIMER)
	    CNET_stop_timer(tid);
	paused = true;
}

bool am_walking(void)
{
	return (paused == false);
}
