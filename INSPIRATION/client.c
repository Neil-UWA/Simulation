#include "client.h"

typedef struct {
    CnetAddr		addr;
    double		rxsignal;
    CnetTime		last_heard;
    CnetTime		last_asked;
} APINFO;

static	APINFO		*APs	= NULL;
	int		nAPs	= 0;

#define	FOREACH_AP	for(a=0, ap=APs ; a<nAPs ; ++a, ++ap)
static	int		a;
static	APINFO		*ap;

typedef struct {
    CnetAddr		with;
    CnetTime		when;
    CnetAddr		asking;
    CnetTimerID		tid;
} ASSOCIATION;

static ASSOCIATION	assoc 	= { UNKNOWN, 0, UNKNOWN, NULLTIMER };


//  ----------------------------------------------------------------

/**
* @brief determine which AP to request an association
*/
static CnetAddr best_AP(void)
{
    CnetTime	recently 	= nodeinfo.time_in_usec - ASSOC_CANCEL_PERIOD;
    APINFO	*best_ap	= NULL;
    double	best_rxsignal	= -1000.0;

    FOREACH_AP {
	if(ap->last_asked < recently) {
	    if(best_rxsignal < ap->rxsignal) {
		best_ap		= ap;
		best_rxsignal	= ap->rxsignal;
	    }
	}
    }
    if(best_ap != NULL) {
	best_ap->last_asked = nodeinfo.time_in_usec;
	return best_ap->addr;
    }
    return UNKNOWN;
}

/**
* @brief discover/remember that we've heard an AP
*/
static void received_beacon(CnetAddr addr, double rxsignal)
{
    FOREACH_AP {
	if(ap->addr == addr)
	    break;			// FOUND A KNOWN AP
    }
    if(a == nAPs) {
	LOG("discovers AP%i\n", addr);
	APs		= realloc(APs, (nAPs+1)*sizeof(*APs));
	ap		= &APs[nAPs];
	memset(ap, 0, sizeof(*ap));
	++nAPs;
	ap->addr	= addr;		// REMEMBER A NEW AP
    }
    ap->rxsignal	= rxsignal;	// UPDATE DETAILS
    ap->last_heard	= nodeinfo.time_in_usec;
}


/**
* @brief give up on recent association request
*/
static EVENT_HANDLER(assoc_cancel_request)
{
    assoc.with		= UNKNOWN;
    assoc.asking	= UNKNOWN;
    assoc.tid		= NULLTIMER;
}

/**
* @brief forget APs that we haven't heard for a while
*/
static EVENT_HANDLER(cleanup_APs)
{
    CnetTime	ancient = nodeinfo.time_in_usec - AP_CLEANUP_FREQ;

    FOREACH_AP {
	if(ap->last_heard < ancient) {
	    if(ap->addr == assoc.with) {
		LOG("disassociates-from AP%i\n", assoc.asking);
		TX(DL_DISASSOCIATION_REQ, assoc.with);
		DRAW_disassociation_ack(nodeinfo.address, assoc.with);
		assoc.with	= UNKNOWN;

		stats[nodeinfo.nodenumber].time_associated +=
				    (nodeinfo.time_in_usec - assoc.when);
	    }
	    LOG("forgets AP%i\n", ap->addr);
	    APs[a]	= APs[nAPs-1];
	    --a;
	    ap		= &APs[a];
	    --nAPs;
	}
    }
    CNET_start_timer(EV_CLEANUP, AP_CLEANUP_FREQ, 0);
}

//  ----------------------------------------------------------------

void DRAW_node_movement(int nowx, int nowy)
{
    if(assoc.with != UNKNOWN)
	DRAW_move_association(nodeinfo.address, assoc.with, nowx, nowy);
}

/**
* @brief  perform case analysis of new frame
*/
static EVENT_HANDLER(received_frame)
{
    FRAME	frame;
    size_t	length	=	sizeof(FRAME);
    int		link;

    CHECK(CNET_read_physical(&link, &frame, &length));

    switch (frame.header.kind) {
    case DL_ASSOCIATION_REQ :
    case DL_DISASSOCIATION_ACK :
    case DL_DATA :
	break;			// IGNORE ALL OF THESE

    case DL_BEACON : {
	double	rxsignal;

	CHECK(CNET_wlan_arrival(link, &rxsignal, NULL));
	received_beacon(frame.header.src, rxsignal);

	if(assoc.with == UNKNOWN && assoc.asking == UNKNOWN) {
	    assoc.asking	= best_AP();
	    
	    if(assoc.asking != UNKNOWN) {
		LOG("requests AP%i\n", assoc.asking);
		TX(DL_ASSOCIATION_REQ, assoc.asking);
		assoc.tid = CNET_start_timer(EV_ASSOC_CANCEL, ASSOC_CANCEL_PERIOD, 0);
	    }
	}
	break;
      }

    case DL_ASSOCIATION_ACK :
	if(assoc.asking == frame.header.src) {
	    LOG("joins AP%i\n", frame.header.src);
	    assoc.with		= frame.header.src;
	    assoc.when		= nodeinfo.time_in_usec;
	    assoc.asking	= UNKNOWN;
	    if(assoc.tid != NULLTIMER) {
		CNET_stop_timer(assoc.tid);
		assoc.tid	= NULLTIMER;
	    }
	    DRAW_new_association(nodeinfo.address, assoc.with,
				frame.header.src_position.x, frame.header.src_position.y);

	    CNET_start_timer(EV_DATA, DATA_FREQ, 0);
	}
	break;

    case DL_ASSOCIATION_NACK :
	if(assoc.asking == frame.header.src) {
	    LOG("rejected-by AP%i\n", frame.header.src);
	    assoc.with		= UNKNOWN;
	    assoc.asking	= UNKNOWN;
	    if(assoc.tid != NULLTIMER) {
		CNET_stop_timer(assoc.tid);
		assoc.tid	= NULLTIMER;
	    }
	}
	break;

    case DL_DISASSOCIATION_REQ :
	if(assoc.with == frame.header.src) {
	    LOG("dropped-by AP%i\n", frame.header.src);
	    DRAW_disassociation_ack(nodeinfo.address, assoc.with);
	    assoc.with	= UNKNOWN;
	    stats[nodeinfo.nodenumber].time_associated +=
				(nodeinfo.time_in_usec - assoc.when);
	}
	break;
    }
}

//  ----------------------------------------------------------------

/**
* @brief  transmit a DATA frame if we're associated
*/
static EVENT_HANDLER(transmit_data)
{
    if(assoc.with != UNKNOWN) {
	TX(DL_DATA, assoc.with);
	CNET_start_timer(EV_DATA, DATA_FREQ, 0);
    }
}

//  ----------------------------------------------------------------

void init_client(void)
{
    init_walking();
    start_walking();   

    CHECK(CNET_set_handler(EV_PHYSICALREADY, received_frame, 0));

    CHECK(CNET_set_handler(EV_CLEANUP, cleanup_APs, 0));
    CNET_start_timer(EV_CLEANUP, AP_CLEANUP_FREQ, 0);

    CHECK(CNET_set_handler(EV_DATA, transmit_data, 0));
    CNET_start_timer(EV_DATA, DATA_FREQ, 0);

    CHECK(CNET_set_handler(EV_ASSOC_CANCEL, assoc_cancel_request, 0));
}
