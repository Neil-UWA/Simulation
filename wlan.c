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
