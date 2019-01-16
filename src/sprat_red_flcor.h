/************************************************************************

 File:				sprat_red_thput.h
 Last Modified Date:     	22/11/2016

************************************************************************/

char SPFLCOR_BLURB_FILE [200];

char SPFLCOR_OUTPUTF [100]	= "spthput.dat";

/* Range over which the spectrum is normalised to unity for the F_lambda plot. 
 * Values here are roughly the V band. This would be easy to move into a config
 * item in the config.ini
 */
int FLAMBDA_NORM_RANGE_LOW  = 5000;
int FLAMBDA_NORM_RANGE_HIGH = 6000;



