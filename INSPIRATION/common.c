#include "simulation.h"

/**
* @brief  send a simple control frame, no payload
*/
void TX(KIND kind, CnetAddr dst)
{
    FRAME	frame;

    frame.header.kind	= kind;
    frame.header.dst	= dst;
    frame.header.src	= nodeinfo.address;	// from me!
    CHECK(CNET_get_position(&frame.header.src_position, NULL));
    frame.header.len	= 0;

    int		link	= 1;
    size_t	length	= sizeof(HEADER);

    CHECK(CNET_write_physical_reliable(link, &frame, &length));
}

/**
* @brief  print annotation to stderr
*/
void LOG(const char *fmt, ...)
{
#if	!defined(NO_LOG)
    va_list	ap;

    if(nodeinfo.nodetype == NT_MOBILE)
	fprintf(stderr, "\t\t\t%02i ", nodeinfo.address);
    else
	fprintf(stderr, "%s ", nodeinfo.nodename);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#endif
}

//  ----------------------------------------------------------------------------

void DRAW_move_association(CnetAddr c, CnetAddr a, int cx, int cy)
{
    TCLTK("scan [$map coords assoc%i_%i] \"%%s %%s %%s %%s\" _ _ x1 y1 ; $map coords assoc%i_%i %i %i $x1 $y1",
		c, a, c, a, SCALE(cx), SCALE(cy) );
}

void DRAW_new_association(CnetAddr c, CnetAddr a, int ax, int ay)
{
    CnetPosition	now;

    CHECK(CNET_get_position(&now, NULL));
    TCLTK("$map create line %i %i %i %i -width 2 -fill darkgreen -tag assoc%i_%i",
		SCALE(now.x), SCALE(now.y), SCALE(ax), SCALE(ay), c, a);
}

void DRAW_disassociation_req(CnetAddr c, CnetAddr a)
{
    TCLTK("$map itemconfig assoc%i_%i -dash {4 4}", c, a);
}

void DRAW_disassociation_ack(CnetAddr c, CnetAddr a)
{
    TCLTK("$map delete assoc%i_%i", c, a);
}
