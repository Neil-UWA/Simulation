#define	map_width		600
#define	map_height		1200

#define FREE		0
#define	OBSTACLE	1
#define	COVERED		2

extern void init_map(void);
extern void set_barrier(OBJECT obj);
extern int	get_building_area(void);
extern int	outdoor_area(void);
extern void set_covered(CnetPosition current);
extern int	outdoor_coverage(void);
extern double coverage_rate(void);
