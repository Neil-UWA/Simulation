#include <cnet.h>
#include "common.h"
#include "mapping.h"
#include "coverage.h"

static int	map[map_width][map_height];

/**
* @brief initialize the map, setting all units to be walkable
*/
void init_map(void){
	for (int i = 0; i < map_width; i++) {
		for (int j = 0; j < map_height; j++) {
			map[i][j]	=	FREE;
		}
	}
}

/**
* @brief set barriers, like buildings, pond, on the map
*
* @param obj
*/
void set_barrier(OBJECT obj){
	int	width, height;

	width	=	obj.x1 - obj.x0;
	height	=	obj.y1 - obj.y0;

	for (int i = 0; i < width; i++) {
		for (int j= 0; j < height; j++) {
			map[(int)obj.x0+i][(int)obj.y0+j] = OBSTACLE;
		}
	}
}

/**
* @brief calculate the total area of buildings
*
* @return 
*/
int	get_building_area(void){
	int	area = 0;
	for (int i = 0; i < map_width; i++) {
		for (int j = 0; j < map_height; j++) {
			if (map[i][j] == OBSTACLE) {
				area += 1;
			}
		}
	}

	return area;
}

/**
* @brief calculate the outdoor area
*
* @return 
*/
int	outdoor_area(void){
	int	total_area =0;
	int	building_area = 0;

	total_area	=	map_width*map_height;
	building_area	=	get_building_area();
	
	return (total_area - building_area);
}

/**
* @brief indicate this spot is covered by APs' signals
*/
void set_covered(CnetPosition current){
	double  mapscale  = CNET_get_mapscale();
	map[current.x][current.y] = COVERED;
	TCLTK("$map create rect %d %d %d %d -width 1 -outline %s -fill %s",
			SCALE(current.x), SCALE(current.y), SCALE(current.x + 1), SCALE(current.y +1),
			COLOUR_OBJECTS, "blue");
}

/**
* @brief get the area of outdoor covered by signals
*
* @return 
*/
int outdoor_coverage(void){
	int	covered_area = 0;
	for (int i = 0; i < map_width; i++) {
		for (int j = 0; j < map_height; j++) {
			if (map[i][j] == COVERED) {
				covered_area += 1;
			}
		}
	}
	printf("Units covered %d\n", covered_area);
	return covered_area;
}

/**
* @brief get the percentage  of outdoor areas covered by signals
*
* @return 
*/
double coverage_rate(void){
	return (double) outdoor_coverage()/(double) outdoor_area();
} 

