#include <cnet.h>
#include <time.h>
#include <string.h>

#include "mapping.h"
#include "walking.h"

#define	EV_TALKING	EV_TIMER1
#define EV_BEACON	EV_TIMER2

#define MAX_SIZE 50

//USER DEFINED WLAN MODEL
static WLANRESULT my_WLAN_model(WLANSIGNAL *sig);

//static char message[] = "hello";
static char beacon[] = "I am a beacon frame";

typedef struct{
	char	message[MAX_SIZE];
	//int		nodetype; //TYPE OF THE NODE
	int		nodenumber;
}package;

EVENT_HANDLER(click){
  time_t  now;
  time(&now);
  printf("hello\n");    
  printf("%s\n", ctime(&now));
  CNET_start_timer(EV_TALKING, (CnetTime)2000000, 0);
}

EVENT_HANDLER(ap){
  static package frame;
  size_t	len = sizeof(frame);

  frame.nodenumber = nodeinfo.nodenumber;
  strcpy(frame.message,beacon);
  printf("I am an accesspoint with one wlan link\n");

  CNET_write_physical_reliable(1,&frame,&len);
  CNET_start_timer(EV_BEACON, (CnetTime)2000000, 0);
}

EVENT_HANDLER(reboot_node){
  char**	argv = (char**) data;

  if(argv[0]){

	//  READ AND DRAW THE MAP
    readmap(argv[0]);

	CNET_set_wlan_model(my_WLAN_model);

	//  WE REQUIRE EACH NODE TO HAVE A DIFFERENT STREAM OF RANDOM NUMBERS
	CNET_srand(nodeinfo.time_of_day.sec + nodeinfo.nodenumber);

    //FUNCTIONS FOR ACCESS POINTS
    if(nodeinfo.nodetype == NT_ACCESSPOINT) {
	 // init_walking();
	 // start_walking();
      CNET_set_handler(EV_BEACON, ap, 0);
      CNET_start_timer(EV_BEACON, (CnetTime)1000000, 0);
    }

    //FUNCTIONS FOR MOBILE CLIENTS
    if(nodeinfo.nodetype == NT_MOBILE) {
	  init_walking();
	  start_walking();
      CNET_set_handler(EV_TALKING, click, 0);
      CNET_start_timer(EV_TALKING, (CnetTime)1000000, 0);
    }
  }
}

#define	SIGNAL_LOSS_PER_OBJECT		12.0		// dBm

static WLANRESULT my_WLAN_model(WLANSIGNAL *sig)
{
    int		dx, dy;
    double	metres;
    double	TXtotal, FSL, budget;

//  CALCULATE THE TOTAL OUTPUT POWER LEAVING TRANSMITTER
    TXtotal	= sig->tx_info->tx_power_dBm - sig->tx_info->tx_cable_loss_dBm +
		    sig->tx_info->tx_antenna_gain_dBi;

//  CALCULATE THE DISTANCE TO THE DESTINATION NODE
    dx		= (sig->tx_pos.x - sig->rx_pos.x);
    dy		= (sig->tx_pos.y - sig->rx_pos.y);
    metres	= sqrt((double)(dx*dx + dy*dy)) + 0.1;	// just 2D

//  CALCULATE THE FREE-SPACE-LOSS OVER THIS DISTANCE
    FSL		= (92.467 + 20.0*log10(sig->tx_info->frequency_GHz)) +
		    20.0*log10(metres/1000.0);

//  CALCULATE THE SIGNAL STRENGTH ARRIVING AT RECEIVER
    sig->rx_strength_dBm = TXtotal - FSL +
	    sig->rx_info->rx_antenna_gain_dBi - sig->rx_info->rx_cable_loss_dBm;

//  DEGRAGDE THE WIRELESS SIGNAL BASED ON THE NUMBER OF OBJECTS IT HITS
    int	nobjects = through_N_objects(sig->tx_pos, sig->rx_pos);

    sig->rx_strength_dBm -= (nobjects * SIGNAL_LOSS_PER_OBJECT);

//  CAN THE RECEIVER DETECT THIS SIGNAL AT ALL?
    budget	= sig->rx_strength_dBm - sig->rx_info->rx_sensitivity_dBm;
    if(budget < 0.0)
	return(WLAN_TOOWEAK);

//  CAN THE RECEIVER DECODE THIS SIGNAL?
    return (budget < sig->rx_info->rx_signal_to_noise_dBm) ?
		WLAN_TOONOISY : WLAN_RECEIVED;
}
