#include "simulation.h"
#include "accesspoint.h"
#include "client.h"   

#if	!defined(USE_BASIC_WLAN_MODEL) && !defined(USE_FSL_WLAN_MODEL)
#define	USE_BASIC_WLAN_MODEL		1
#endif

#if	!defined(SIGNAL_LOSS_PER_OBJECT)
#define SIGNAL_LOSS_PER_OBJECT		12.0      // dBm
#endif

STATS		*stats			= NULL;
double		mapscale;

// ----------------------------------------------------------------------

#if	defined(USE_FSL_WLAN_MODEL)
static WLANRESULT fsl_WLAN_model(WLANSIGNAL *sig)
{
    int       dx, dy;
    double  metres;
    double  TXtotal, FSL, budget;

    //  CALCULATE THE TOTAL OUTPUT POWER LEAVING TRANSMITTER
    TXtotal = sig->tx_info->tx_power_dBm - sig->tx_info->tx_cable_loss_dBm +
        sig->tx_info->tx_antenna_gain_dBi;

    //  CALCULATE THE DISTANCE TO THE DESTINATION NODE
    dx        = (sig->tx_pos.x - sig->rx_pos.x);
    dy        = (sig->tx_pos.y - sig->rx_pos.y);
    metres  = sqrt((double)(dx*dx + dy*dy)) + 0.1;  // just 2D

    //  CALCULATE THE FREE-SPACE-LOSS OVER THIS DISTANCE
    FSL       = (92.467 + 20.0*log10(sig->tx_info->frequency_GHz)) +
        20.0*log10(metres/1000.0);

    //  CALCULATE THE SIGNAL STRENGTH ARRIVING AT RECEIVER
    sig->rx_strength_dBm = TXtotal - FSL +
        sig->rx_info->rx_antenna_gain_dBi - sig->rx_info->rx_cable_loss_dBm;

    //  DEGRAGDE THE WIRELESS SIGNAL BASED ON THE NUMBER OF OBJECTS IT HITS
    int   nobjects = through_N_objects(sig->tx_pos, sig->rx_pos);

    sig->rx_strength_dBm -= (nobjects * SIGNAL_LOSS_PER_OBJECT);

    //  CAN THE RECEIVER DETECT THIS SIGNAL AT ALL?
    budget  = sig->rx_strength_dBm - sig->rx_info->rx_sensitivity_dBm;
    if(budget < 0.0)
        return(WLAN_TOOWEAK);

    //  CAN THE RECEIVER DECODE THIS SIGNAL?
    return (budget < sig->rx_info->rx_signal_to_noise_dBm) ?
        WLAN_TOONOISY : WLAN_RECEIVED;
}

#elif	defined(USE_BASIC_WLAN_MODEL)
static WLANRESULT basic_WLAN_model(WLANSIGNAL *sig)
{
    int		dx, dy;
    double	metres;

    //  CALCULATE THE DISTANCE TO THE DESTINATION NODE
    dx        = (sig->tx_pos.x - sig->rx_pos.x);
    dy        = (sig->tx_pos.y - sig->rx_pos.y);
    metres  = sqrt((double)(dx*dx + dy*dy)) + 0.1;  // just 2D

    //  CAN THE RECEIVER HEAR THIS SIGNAL?
    if(metres < BASIC_TX_RADIUS)
        return WLAN_RECEIVED;

    return WLAN_TOOWEAK;
}
#endif

// ----------------------------------------------------------------------

EVENT_HANDLER(shutdown)
{
    CnetTime	total	 = 0;
    int		nclients = 0;

    for(int n=0 ; n<NNODES ; ++n)
	if(stats[n].time_associated != 0) {
/*
	    fprintf(stderr, "%02i: %.1lf%%\n",
			n, stats[n].time_associated*100.0 / nodeinfo.time_in_usec);
 */
	    total += stats[n].time_associated;
	    ++nclients;
	}
    if(total != 0)
	fprintf(stderr, "average association %.1lf%%\n",
			(total*100.0)/nclients / nodeinfo.time_in_usec);
}

EVENT_HANDLER(reboot_node)
{
    char **argv = (char **)data;

//  ENSURE THAT WE'LL BE ABLE TO ACCUMULATE STATISTICS
    if(NNODES == 0) {
	fprintf(stderr, "cnet must be invoked with the -N option\n");
	exit(EXIT_FAILURE);
    }

    if(argv[0]) {
//  WE REQUIRE EACH NODE TO HAVE A DIFFERENT STREAM OF RANDOM NUMBERS
        CNET_srand(nodeinfo.time_of_day.sec + nodeinfo.nodenumber);

//  ALL NODES READ THE MAP
		read_map(argv[0]);
		mapscale	= CNET_get_mapscale();

//  ESTABLISH A SHARED-MEMORY SEGMENT FOR ALL NODES' STATISTICS
	stats		= CNET_shmem(NNODES * sizeof(*stats));
	stats[nodeinfo.nodenumber].time_associated	= 0;

#if	defined(USE_BASIC_WLAN_MODEL)
	CNET_set_wlan_model(basic_WLAN_model);
#elif	defined(USE_FSL_WLAN_MODEL)
	CNET_set_wlan_model(fsl_WLAN_model);
#endif

        if(nodeinfo.nodetype == NT_ACCESSPOINT)
            init_accesspoint();
        else
            init_client();

	if(nodeinfo.nodenumber == 0) {
	    draw_map();
	    TCLTK("$map create text 70 2 -anchor nw -text \"MAX_CLIENTS_PER_AP=%i\"",
			MAX_CLIENTS_PER_AP);
	    CHECK(CNET_set_handler(EV_SHUTDOWN, shutdown, 0));
	}
    }
}
