#include <cnet.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "mapping.h"

#define	UNKNOWN			(-1)

static	OBJECT	*objects	= NULL;
static	int	nobjects	= 0;
static	OBJECT	**paths		= NULL;
static	int	npaths		= 0;

#define	FOREACH_OBJECT	for(n=0, op=objects ; n<nobjects ; ++n, ++op)

static	CnetPosition	mapsize;
static double		mapscale;


//  ---------------------------------------------------------------------------

static void add_object(OBJTYPE type, const char *str,
			double x0, double y0, double x1, double y1)
{
    objects	= realloc(objects, (nobjects+1)*sizeof(OBJECT));

    if(objects) {
        extern char *strdup(const char *str);

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

void draw_walk(CnetPosition *now, CnetPosition *newdest)
{
    TCLTK("$map create line %d %d %d %d -fill red -width 3",
	SCALE(now->x), SCALE(now->y), SCALE(newdest->x), SCALE(newdest->y) );
}

void draw_map(void)
{
    int		n;
    OBJECT	*op;

    TCLTK("font create bfont -family Helvetica -size 14 -weight normal");
    FOREACH_OBJECT {
	switch (op->type) {
	case T_OBJ:
	    TCLTK("$map create rect %d %d %d %d -outline %s -fill %s",
		    SCALE(op->x0), SCALE(op->y0), SCALE(op->x1), SCALE(op->y1),
		    COLOUR_OBJECTS, COLOUR_OBJECTS);
	    break;
	case T_TEXT:
	    TCLTK("$map create text %d %d -font bfont -text \"%s\"",
		    SCALE(op->x0), SCALE(op->y0),
		    op->str);
	    break;
	case T_POINT:
	    break;
	case T_PATH:
	    TCLTK("$map create rect %d %d %d %d -outline %s -fill %s",
		    SCALE(op->x0-2), SCALE(op->y0-2), SCALE(op->x1+2), SCALE(op->y1+2),
		    COLOUR_PATH, COLOUR_PATH);
	    break;
	default:
	    break;
        }
    }
}

static bool find_point(const char *str, double *x, double *y)
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

void read_map(const char *mapfile)
{
    FILE *fp	= fopen(mapfile, "r");

//  EACH NODE READS IN THE MAP DETAILS
    if(fp) {
        char	line[1024], str0[1024], str1[1024], *s;

        CNET_get_position(NULL, &mapsize);
        mapscale = CNET_get_mapscale();

        while(fgets(line, sizeof(line), fp)) {
            s	= trim(line);

            if(*s) {
                double	x0, y0, x1, y1;

                if(sscanf(s, "object %lf %lf %lf %lf", &x0, &y0, &x1, &y1) == 4)
                    add_object(T_OBJ, NULL, x0, y0, x1, y1);
                else if(sscanf(s, "text %lf %lf %s", &x0, &y0, str0) == 3)
                    add_object(T_TEXT, str0, x0, y0, 0, 0);
                else if(sscanf(s, "point %lf %lf %s", &x0, &y0, str0) == 3)
                    add_object(T_POINT, str0, x0, y0, 0, 0);
                else if(sscanf(s, "path %lf %lf %lf %lf", &x0, &y0, &x1, &y1) == 4)
		    add_object(T_PATH, NULL, x0, y0, x1, y1);
                else if(sscanf(s, "path %s %s", str0, str1) == 2) {
		    if(find_point(str0, &x0, &y0) && find_point(str1, &x1, &y1))
			add_object(T_PATH, NULL, x0, y0, x1, y1);
		}
            }
        }
        fclose(fp);

//  FIND AND CACHE ALL OF THE PATHS
	int	n, np=0;
	OBJECT	*op;

	paths = malloc(npaths * sizeof(*paths));
	FOREACH_OBJECT {
	    if(op->type == T_PATH)
		paths[np++] = op;
	}
    }
    else {
        fprintf(stderr, "%s: cannot open '%s'\n", nodeinfo.nodename, mapfile);
        exit(EXIT_FAILURE);
    }
}

static bool intersect(	double x0, double y0, double x1, double y1,
			double x2, double y2, double x3, double y3)
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


//  DOES THE PATH FROM S -> D PASS THROUGH AN OBJECT?
bool through_an_object(CnetPosition S, CnetPosition D)
{
    int		n;
    OBJECT	*op;

    FOREACH_OBJECT {
	if(op->type == T_OBJ)		// only interested in objects
	    if(	intersect(S.x, S.y, D.x, D.y, op->x0, op->y0, op->x1, op->y0) ||
		intersect(S.x, S.y, D.x, D.y, op->x1, op->y0, op->x1, op->y1) ||
		intersect(S.x, S.y, D.x, D.y, op->x1, op->y1, op->x0, op->y1) ||
		intersect(S.x, S.y, D.x, D.y, op->x0, op->y1, op->x0, op->y0) )
		return true;
    }
    return false;
}

//  THROUGH HOW MANY OBJECTS DOES THE PATH FROM S -> D PASS?
int through_N_objects(CnetPosition S, CnetPosition D)
{
    int		n;
    OBJECT	*op;
    int	count	= 0;

    FOREACH_OBJECT {
	if(op->type == T_OBJ)		// only interested in objects
	    if(	intersect(S.x, S.y, D.x, D.y, op->x0, op->y0, op->x1, op->y0) ||
		intersect(S.x, S.y, D.x, D.y, op->x1, op->y0, op->x1, op->y1) ||
		intersect(S.x, S.y, D.x, D.y, op->x1, op->y1, op->x0, op->y1) ||
		intersect(S.x, S.y, D.x, D.y, op->x0, op->y1, op->x0, op->y0) )
		++count;
    }
    return count;
}

//  CHOOSE A POSITION THAT IS OUTSIDE OF ALL OBJECTS
void choose_outside_position(CnetPosition *new, int maxdist)
{
    int mdmd	= (maxdist * maxdist);

    for(;;) {
	double	tryx	= CNET_rand() % mapsize.x;
	double	tryy	= CNET_rand() % mapsize.y;

//  IF REQUESTED, ENSURE THAT NEW POINT IS CLOSE ENOUGH TO THE OLD POINT
	if(mdmd != 0 &&
	 ((tryx-new->x)*(tryx-new->x) + (tryy-new->y)*(tryy-new->y)) > mdmd)
	    continue;

	int	n;
	OBJECT	*op;

	FOREACH_OBJECT {
	    if(op->type == T_OBJ &&
		tryx >= op->x0 && tryx <= op->x1 &&
		tryy >= op->y0 && tryy <= op->y1)
		    break;		// oops, inside an object
	}
	if(n == nobjects) {
	    new->x	= tryx;
	    new->y	= tryy;
	    break;
	}
    }
    new->z	= 0;
}

//  CHOOSE A POSITION ON ANY PATH CONNECTED TO THE CURRENT POSITION/PATH
void choose_path_position(CnetPosition *new, CnetPosition position)
{
//  IF JUST REBOOTED, CHOOSE A RANDOM PATH+END FROM WHICH TO START
    if(position.x == 0 && position.y == 0) {
	int p	= CNET_rand() % npaths;

	if((CNET_rand() % 2) == 0) {
	    new->x	= paths[p]->x0;
	    new->y	= paths[p]->y0;
	}
	else {
	    new->x	= paths[p]->x1;
	    new->y	= paths[p]->y1;
	}
    }

//  ELSE, FIND THE PATH+END TO WHICH WE ARE CURRENTLY THE CLOSEST
    else {
	OBJECT	*best	= NULL;
	int	e	= UNKNOWN;
	double	mindist	= (1<<20);
	double	dx, dy, d;
	int	p	= CNET_rand() % npaths;

	for(int n=0 ; n<npaths ; ++n) {
	    p	= (p+1)%npaths;

	    OBJECT *op	= paths[p];

	    dx	= (op->x0 - position.x);
	    dy	= (op->y0 - position.y);
	    d	= dx*dx + dy*dy;
	    if(mindist > d) {
		mindist	= d;
		best	= op;
		e	= 0;
	    }

	    dx	= (op->x1 - position.x);
	    dy	= (op->y1 - position.y);
	    d	= dx*dx + dy*dy;
	    if(mindist > d) {
		mindist	= d;
		best	= op;
		e	= 1;
	    }
	}
//  AND SET THE PATH'S OTHER END AS OUR NEXT DESTINATION
	if(e == 0) {
	    new->x	= best->x1;
	    new->y	= best->y1;
	}
	else {
	    new->x	= best->x0;
	    new->y	= best->y0;
	}
    }
    new->z	= 0;
}

/** 
 * @brief Am I inside the given object
 * @param CnetPosition position 
 * @param OBJECT object  
 * @return bool true if current position is inside the given object 
 */
bool inside(CnetPosition position, OBJECT object){
	return (position.x>=object.x0&&position.x<=object.x1
			&&position.y>=object.y0&&position.y<=object.y1);
}

/** 
 * @brief randomly choose a new destination inside the given
 * object 
 * @param CnetPosition *newdest
 * @param OBJECT *object
 * return 
 */
void random_choose(CnetPosition *newdest, OBJECT *object){
	memset(newdest, 0, sizeof(CnetPosition));

	do{
	  newdest->x  = CNET_rand() % mapsize.x;
	  newdest->y  = CNET_rand() % mapsize.y;
	}while(!inside(*newdest,*object));
}

/** 
 * @brief which object am I in
 * @param CnetPosition position
 * @param OBJECT *temp
 * @return bool true if found the object am i in.
 */
bool insideObject(CnetPosition position, OBJECT *temp){
	OBJECT	*op;
	int		n;
	bool	found = false;// haven't found the object 

	FOREACH_OBJECT{
		if(inside(position, op[n])){
			memcpy(temp, &op[n], sizeof(OBJECT));
			found = true;
			break;
		}
	}

	return found;
}
