#include "accesspoint.h"

typedef struct {
    CnetAddr		addr;
    double		rxsignal;
    CnetTime		last_heard;
} CLIENTINFO;

static	CLIENTINFO	*clients	= NULL;
static	int		nclients	= 0;

#define	FOREACH_CLIENT	for(c=0, cp=clients ; c<nclients ; ++c, ++cp)
static	int		c;
static	CLIENTINFO	*cp;


/**
* @brief  sending beacon frames 10 times every second
*/
static EVENT_HANDLER(transmit_beacon)
{
    TX(DL_BEACON, BROADCAST);
    CNET_start_timer(EV_BEACON, BEACON_FREQ,  0);
}

static void DRAW_AP_full(void)
{
    CnetPosition now;

    CHECK(CNET_get_position(&now, NULL));
    TCLTK("$map create oval %d %d %d %d -dash {6 6} -width 2 -outline %s -tag APfull%i",
		SCALE(now.x)-30, SCALE(now.y)-30,
		SCALE(now.x)+30, SCALE(now.y)+30,
		"red", nodeinfo.address);
}

static void DRAW_AP_available(void)
{
    TCLTK("$map delete APfull%i", nodeinfo.address);
}

//  -------------------------------------------------------------------

/**
* @brief  disassociate from clients that we haven't heard from
*/
static EVENT_HANDLER(cleanup_clients)
{
    CnetTime	ancient = nodeinfo.time_in_usec - CLIENT_CLEANUP_FREQ;
    int		nwas = nclients;

    FOREACH_CLIENT {
	if(cp->last_heard < ancient) {
	    LOG("drops %02i\n", cp->addr);
	    TX(DL_DISASSOCIATION_REQ, cp->addr);
	    DRAW_disassociation_req(cp->addr, nodeinfo.address);
	    clients[c] = clients[nclients-1];
	    --c;
	    cp	= &clients[c];
	    --nclients;
	}
    }
    if(nwas == MAX_CLIENTS_PER_AP && nclients < nwas)
	DRAW_AP_available();
    CNET_start_timer(EV_CLEANUP, CLIENT_CLEANUP_FREQ, 0);
}

/**
* @brief  remember that we've recently heard this client
*/
static void refresh_client(CnetAddr addr)
{
    FOREACH_CLIENT {
	if(cp->addr == addr) {
	    cp->last_heard	= nodeinfo.time_in_usec;
	    break;
	}
    }
}

/**
* @brief  associate new clients, if we have capacity
*/
static bool associate_client(CnetAddr addr)
{
    FOREACH_CLIENT {
	if(cp->addr == addr)
	    return true;	// ALREADY ASSOCIATED
    }

    if(nclients == MAX_CLIENTS_PER_AP) {
	LOG("rejects %02i\n", addr);
	return false;		// ALREADY AT CAPACITY
    }

    LOG("associates %02i\n", addr);
    clients		= realloc(clients, (nclients+1)*sizeof(*clients));
    cp			= &clients[nclients];
    ++nclients;
    memset(cp, 0, sizeof(*cp));
    cp->addr		= addr;
    cp->last_heard	= nodeinfo.time_in_usec;

    if(nclients == MAX_CLIENTS_PER_AP)
	DRAW_AP_full();
    return true;		// YES, ASSOCIATE NEW CLIENT
}

/**
* @brief  disassociate existing clients
*/
static bool disassociate_client(CnetAddr addr)
{
    FOREACH_CLIENT {
	if(cp->addr == addr) {
	    LOG("disassociates %02i\n", addr);
	    clients[c] = clients[nclients-1];
	    --nclients;
	    return true;	// WAS KNOWN, NOW DISASSOCIATED
	}
    }
    return false;		// WAS NOT ASSOCIATED
}

//  -------------------------------------------------------------------

/**
* @brief  perform case analysis of new frame
*/
static EVENT_HANDLER(receive_frame)
{
    FRAME	frame;
    size_t	length = sizeof(FRAME);
    int		link;

    CHECK(CNET_read_physical(&link, &frame, &length));
    
    switch (frame.header.kind) {
    case DL_BEACON :
    case DL_ASSOCIATION_ACK :
    case DL_ASSOCIATION_NACK :
    case DL_DISASSOCIATION_ACK :
	break;			// IGNORE ALL OF THESE

    case DL_DATA :
	if(FOR_ME(frame))
	    refresh_client(frame.header.src);
	break;

    case DL_ASSOCIATION_REQ :
	if(FOR_ME(frame)) {
	    if(associate_client(frame.header.src))
		TX(DL_ASSOCIATION_ACK,  frame.header.src);
	    else
		TX(DL_ASSOCIATION_NACK, frame.header.src);
	}
	break;

    case DL_DISASSOCIATION_REQ :
	if(FOR_ME(frame)) {
	    disassociate_client(frame.header.src);
	    DRAW_disassociation_ack(frame.header.src, nodeinfo.address);
	    TX(DL_DISASSOCIATION_ACK, frame.header.src);
	}
	break;
    }
}

//  -------------------------------------------------------------------

void init_accesspoint(void)
{
    CnetPosition now;

    CHECK(CNET_get_position(&now, NULL));
    TCLTK("$map create oval %d %d %d %d -width 2 -outline %s",
	    SCALE(now.x-BASIC_TX_RADIUS), SCALE(now.y-BASIC_TX_RADIUS),
	    SCALE(now.x+BASIC_TX_RADIUS), SCALE(now.y+BASIC_TX_RADIUS),
	    "purple");

    CHECK(CNET_set_handler(EV_BEACON, transmit_beacon, 0));    
    CNET_start_timer(EV_BEACON, BEACON_FREQ + CNET_rand()%10000,  0);

    CHECK(CNET_set_handler(EV_PHYSICALREADY, receive_frame, 0));

    CHECK(CNET_set_handler(EV_CLEANUP, cleanup_clients, 0));
    CNET_start_timer(EV_CLEANUP, CLIENT_CLEANUP_FREQ, 0);
}
