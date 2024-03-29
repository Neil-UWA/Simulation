#include <cnet.h>
#include <time.h>
#include <string.h>

#include "common.h"
#include "client.h"   
#include "accesspoint.h"
#include "mapping.h"
#include "walking.h"
#include "am.h"
#include "coverage.h"

#define	SIGNAL_LOSS_PER_OBJECT		12.0		// dBm

//USER DEFINED WLAN MODEL
//static WLANRESULT my_WLAN_model(WLANSIGNAL *sig);

/* ---------------------------------------------------------------------- */

EVENT_HANDLER(reboot_node){
	char**	argv = (char**) data;

	if(argv[0]){

		//  READ AND DRAW THE MAP
		readmap(argv[0]);

//		CNET_set_wlan_model(my_WLAN_model);

		//  WE REQUIRE EACH NODE TO HAVE A DIFFERENT STREAM OF RANDOM NUMBERS
		CNET_srand(nodeinfo.time_of_day.sec + nodeinfo.nodenumber);
		printf("%s\n", ctime(&nodeinfo.time_of_day.sec));

		if (nodeinfo.nodenumber == 0 && 
		nodeinfo.nodetype == NT_ACCESSPOINT) {
			printf("building area %d\n",get_building_area() );
			printf("outdoor area %d\n",outdoor_area() );
			CHECK(CNET_set_handler(EV_PHYSICALREADY, monitor, 0));
		}

		if(nodeinfo.nodetype == NT_ACCESSPOINT && 
			nodeinfo.nodenumber != 0){
			//CNET_set_handler(EV_TIMER9, walker, 0);
			//CNET_start_timer(EV_TIMER9, 1000000, 0);
			init_beacon();
			start_beacon();
			CHECK(CNET_set_handler(EV_PHYSICALREADY,	listenning, 0));

		}
		
		if (nodeinfo.nodetype == NT_MOBILE) {
//			init_walking();
//			start_walking();     

			//init_talking();
			//start_talking();

			CHECK(CNET_set_handler(EV_PHYSICALREADY,	searching_ap, 0));
		}                                                         
	}
}

/* ---------------------------------------------------------------------- */

/* USER DEFINED WLAN MODE */
//static WLANRESULT my_WLAN_model(WLANSIGNAL *sig)
//{
//	int		dx, dy;
//	double	metres;
//	double	TXtotal, FSL, budget;
//
//	//  CALCULATE THE TOTAL OUTPUT POWER LEAVING TRANSMITTER
//	TXtotal	= sig->tx_info->tx_power_dBm - sig->tx_info->tx_cable_loss_dBm +
//		sig->tx_info->tx_antenna_gain_dBi;
//
//	//  CALCULATE THE DISTANCE TO THE DESTINATION NODE
//	dx		= (sig->tx_pos.x - sig->rx_pos.x);
//	dy		= (sig->tx_pos.y - sig->rx_pos.y);
//	metres	= sqrt((double)(dx*dx + dy*dy)) + 0.1;	// just 2D
//
//	//  CALCULATE THE FREE-SPACE-LOSS OVER THIS DISTANCE
//	FSL		= (92.467 + 20.0*log10(sig->tx_info->frequency_GHz)) +
//		20.0*log10(metres/1000.0);
//
//	//  CALCULATE THE SIGNAL STRENGTH ARRIVING AT RECEIVER
//	sig->rx_strength_dBm = TXtotal - FSL +
//		sig->rx_info->rx_antenna_gain_dBi - sig->rx_info->rx_cable_loss_dBm;
//
//	//  DEGRAGDE THE WIRELESS SIGNAL BASED ON THE NUMBER OF OBJECTS IT HITS
//	int	nobjects = through_N_objects(sig->tx_pos, sig->rx_pos);
//
//	sig->rx_strength_dBm -= (nobjects * SIGNAL_LOSS_PER_OBJECT);
//
//	//  CAN THE RECEIVER DETECT THIS SIGNAL AT ALL?
//	budget	= sig->rx_strength_dBm - sig->rx_info->rx_sensitivity_dBm;
//	if(budget < 0.0)
//		return(WLAN_TOOWEAK);
//
//	//  CAN THE RECEIVER DECODE THIS SIGNAL?
//	return (budget < sig->rx_info->rx_signal_to_noise_dBm) ?
//		WLAN_TOONOISY : WLAN_RECEIVED;
//}
