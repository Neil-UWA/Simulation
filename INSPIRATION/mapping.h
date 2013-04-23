//#include <cnet.h>

#define	COMMENT		'#'
#define	COLOUR_OBJECTS	"#aaaaaa"
#define	COLOUR_PATH	"#80CC80"
#define	SCALE(p)	((int)((p) / mapscale))

typedef enum {
    T_OBJ	= 0,
    T_TEXT,
    T_POINT,
    T_PATH
} OBJTYPE;

typedef struct {
    OBJTYPE	type;
    char	*str;
    double	x0;
    double	y0;
    double	x1;
    double	y1;
} OBJECT;

extern	char	*trim(char *line);
//  READ THE OBJECTS ON OUR MAP FROM THE INDICATED FILE
extern	void	read_map(const char *filenm);

//  ONLY ONE NODE NEEDS TO DRAW THE MAP (typically node0)
extern	void	draw_map(void);

//  CHOOSE A POSITION THAT IS OUTSIDE OF ALL OBJECTS
extern	void	choose_outside_position(CnetPosition *new, int maxdist);

//  CHOOSE A POSITION ON ANY PATH CONNECTED TO THE CURRENT POSITION/PATH
extern	void	choose_path_position(CnetPosition *new, CnetPosition position);

//  DOES THE PATH FROM S -> D PASS THROUGH AN OBJECT?
extern	bool	through_an_object(CnetPosition S, CnetPosition D);

//  THROUGH HOW MANY OBJECTS DOES THE PATH FROM S -> D PASS?
extern	int	through_N_objects(CnetPosition S, CnetPosition D);

//  DRAWS THE PATH TO BE TALEN BY A NODE
extern void	draw_walk(CnetPosition *now, CnetPosition *newdest);

//	AM I INSIDE AN OBJECT
extern bool inside(CnetPosition position, OBJECT object);

//	WHICH OBJECT AM I IN
extern bool insideObject(CnetPosition position, OBJECT *temp);

//	RANDOMLY CHOOSE A DESTINATION INSIDE THE GIVEN OBJECT
extern void random_choose(CnetPosition *newdest, OBJECT *object);
