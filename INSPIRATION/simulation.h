#ifndef	_SIMULATION_H_
#define	_SIMULATION_H_

#include <cnet.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "mapping.h"

#if	!defined(MAX_PAYLOAD)
#define	MAX_PAYLOAD		1024
#endif

//  ---------------------------------------------------------------------

#define	UNKNOWN			(-1)
#define	BROADCAST		(-1)	// broadcasting 

#define EV_BEACON		EV_TIMER1
#define EV_DATA			EV_TIMER2
#define	EV_ASSOC_CANCEL		EV_TIMER3
#define	EV_CLEANUP		EV_TIMER4

/**
* @brief types of frame
*/
typedef enum {
	DL_BEACON,

	DL_ASSOCIATION_REQ,
	DL_ASSOCIATION_ACK,
	DL_ASSOCIATION_NACK,

	DL_DISASSOCIATION_REQ,
	DL_DISASSOCIATION_ACK,

	DL_DATA
} KIND; 

typedef struct {
	KIND		kind;
	CnetAddr	dst;
	CnetAddr	src;
	CnetPosition	src_position;
	size_t		len;		// length of the payload
} HEADER;

#define	FOR_ME(frame)	(frame.header.dst == nodeinfo.nodenumber)

typedef struct {
	HEADER		header;
	char		payload[MAX_PAYLOAD];
} FRAME;

typedef struct {
	CnetTime	time_associated;
} STATS;

extern	void	TX(KIND kind, CnetAddr dst);
extern	void	LOG(const char *fmt, ...);

extern	void	DRAW_node_movement(int nowx, int nowy);
extern	void	DRAW_new_association(CnetAddr c, CnetAddr a, int ax, int ay);
extern	void	DRAW_move_association(CnetAddr c, CnetAddr a, int ax, int ay);
extern	void	DRAW_disassociation_req(CnetAddr c, CnetAddr a);
extern	void	DRAW_disassociation_ack(CnetAddr c, CnetAddr a);

extern	STATS		*stats;
extern	double		mapscale;

#endif
