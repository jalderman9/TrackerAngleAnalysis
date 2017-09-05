/**
 * @file	solarpos.c
 * @author	Jason Alderman
 * @date	04SEP2017
 *
 * @brief
 *   Solar position algorithm
 *
 * @copyright Jason Alderman © 2017 - ALL RIGHTS RESERVED
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>

#include "angle_conversions.h"
#include "solarpos.h"


/**
 * @brief 
 *  Calculates juliand day of the year
 * 
 * @param [in] solarpos_time pointer to solarpos_time_t struct
 * 
 * @return jday Integer julian day of the year
*/
static inline int julian(solarpos_inputs_t *solarpos_inputs) 
{
	int i=1, jday=0, k;
	int nday[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if (solarpos_inputs->year % 4 == 0) 
	{
		k = 1;
	} 
	else 
	{
		k = 0;
	}

	while (i < solarpos_inputs->month) 
	{
		jday = jday + nday[i-1];
		i++;
	}

	if (solarpos_inputs->month > 2) 
	{
		jday = jday + k + solarpos_inputs->day;
	} 
	else 
	{
		jday = jday + solarpos_inputs->day;
	}

	return(jday);
}


/**
 * @brief
 *   Calculate solar position at the given time of day and coordinates
 * 
 *  This function is based on a paper by Michalsky published in Solar Energy
 * Vol. 40, No. 3, pp. 227-235, 1988. It calculates solar position for the
 * time and location passed to the function based on the Astronomical
 * Almanac's Algorithm for the period 1950-2050. For data averaged over an
 * interval, the appropriate time passed is the midpoint of the interval.
 * (Example: For hourly data averaged from 10 to 11, the time passed to the
 * function should be 10 hours and 30 minutes). The exception is when the time
 * interval includes a sunrise or sunset. For these intervals, the appropriate
 * time should be the midpoint of the portion of the interval when the sun is
 * above the horizon. (Example: For hourly data averaged from 7 to 8 with a
 * sunrise time of 7:30, the time passed to the function should be 7 hours and
 * and 45 minutes).
 * 
 * Revised 5/15/98. Replaced algorithm for solar azimuth with one by Iqbal
 * so latitudes below the equator are correctly handled. Also put in checks
 * to allow an elevation of 90 degrees without crashing the program and prevented
 * elevation from exceeding 90 degrees after refraction correction.
 * 
 * This function calls the function julian to get the julian day of year.
 * 
 * List of Parameters Passed to Function:
 * @param [in] solarpos_inputs pointer to solarpos_inputs_t struct with location and time
 */
solarpos_t *solar_position_calc(solarpos_inputs_t *solarpos_inputs) 
{
	double elv, azm, refrac, E, ws, sunrise, sunset, tst;

	int jday = julian(solarpos_inputs);	// Get julian day of year
	double zulu = solarpos_inputs->hour + solarpos_inputs->minute / 60.0 - solarpos_inputs->timezone;	// Convert local time to zulu time
	
	if (zulu < 0.0) 
	{
		zulu = zulu + 24.0; // Force time between 0-24 hrs             
		jday = jday - 1; // Adjust julian day if needed
	}
	else if (zulu > 24.0) 
	{
		zulu = zulu - 24.0;
		jday = jday + 1;
	}
	int delta = solarpos_inputs->year - 1949;
	int leap = delta/4;
	double jd = 32916.5 + delta * 365.0 + leap + jday + zulu / 24.0;
	double time = jd - 51545.0;     		// Time in days referenced from noon 1 Jan 2000

	double mnlong = 280.46 + 0.9856474*time;
	mnlong = fmod(mnlong,360.0);		// Finds floating point remainder
	if (mnlong < 0.0) 
	{
		mnlong = mnlong + 360.0;		// Mean longitude between 0-360 deg
	}

	double mnanom = 357.528 + 0.9856003*time;
	mnanom = fmod(mnanom,360.0);
	if ( mnanom < 0.0 ) 
	{
		mnanom = mnanom + 360.0;
	}
	mnanom = deg2rad(mnanom);			// Mean anomaly between 0-2pi radians

	double eclong = mnlong + 1.915*sin(mnanom) + 0.020*sin(2.0*mnanom);
	eclong = fmod(eclong,360.0);
	if ( eclong < 0.0 ) 
	{
		eclong = eclong + 360.0;
	}
	eclong = deg2rad(eclong);       /* Ecliptic longitude between 0-2pi radians */

	double oblqec = deg2rad( ( 23.439 - 0.0000004*time ) );   /* Obliquity of ecliptic in radians */
	double num = cos(oblqec)*sin(eclong);
	double den = cos(eclong);
	double ra  = atan(num/den);                         /* Right ascension in radians */
	if ( den < 0.0 ) 
	{
		ra = ra + M_PI;
	} 
	else if ( num < 0.0 ) 
	{
		ra = ra + 2.0 * M_PI;
	}

	double dec = asin( sin(oblqec)*sin(eclong) );       /* Declination in radians */

	double gmst = 6.697375 + 0.0657098242*time + zulu;
	gmst = fmod(gmst,24.0);
	if ( gmst < 0.0 ) 
	{
		gmst = gmst + 24.0;         /* Greenwich mean sidereal time in hours */
	}

	double lmst = gmst + solarpos_inputs->longitude/15.0;
	lmst = fmod(lmst,24.0);
	if ( lmst < 0.0 ) 
	{
		lmst = lmst + 24.0;
	}
	lmst = deg2rad( lmst * 15.0 );         /* Local mean sidereal time in radians */

	double ha = lmst - ra;
	if ( ha < -M_PI ) 
	{
		ha = ha + 2 * M_PI;
	} 
	else if( ha > M_PI ) 
	{
		ha = ha - 2 * M_PI;             /* Hour angle in radians between -pi and pi */
	}

	double latrad = deg2rad(solarpos_inputs->latitude);                /* Change latitude to radians */

	double arg = sin(dec)*sin(latrad) + cos(dec)*cos(latrad)*cos(ha);  /* For elevation in radians */
	if ( arg > 1.0 ) 
	{
		elv = M_PI / 2.0;
	} 
	else if ( arg < -1.0 ) 
	{
		elv = -M_PI / 2.0;
	} 
	else 
	{
		elv = asin(arg);
	}

	if ( cos(elv) == 0.0 ) 
	{ 	// Assign azimuth = 180 deg if elv = 90 or -90 
		azm = M_PI;         
	} 
	else 
	{	// For solar azimuth in radians per Iqbal 
		arg = ((sin(elv)*sin(latrad)-sin(dec))/(cos(elv)*cos(latrad))); /* for azimuth */
		
		if( arg > 1.0 )
		{
			azm = 0.0;              /* Azimuth(radians)*/
		}
		else if( arg < -1.0 )
		{
			azm = M_PI;
		}
		else
		{
			azm = acos(arg);
		}

		if( ( ha <= 0.0 && ha >= -M_PI) || ha >= M_PI )
		{
			azm = M_PI - azm;
		}
		else
		{
			azm = M_PI + azm;
		}
	}
		
		

	elv = rad2deg(elv);          /* Change to degrees for atmospheric correction */
	if( elv > -0.56 )
		refrac = 3.51561 * ( 0.1594 + 0.0196 * elv + 0.00002 * pow( elv, 2 ) )/( 1.0 + 0.505 * elv + 0.0845 * pow( elv, 2 ) );
	else
		refrac = 0.56;
	if( elv + refrac > 90.0 )
	{
		elv = deg2rad(90.0);
	}
	else
	{
		elv = deg2rad( ( elv + refrac ) ); /* Atmospheric corrected elevation(radians) */
	}

	E = ( mnlong - rad2deg(ra) ) / 15.0;       /* Equation of time in hours */
	
	if( E < - 0.33 )   /* Adjust for error occuring if mnlong and ra are in quadrants I and IV */
	{
		E = E + 24.0;
	}
	else if( E > 0.33 )
	{
		E = E - 24.0;
	}

	arg = -tan(latrad)*tan(dec);
	
	if (arg >= 1.0)
	{
		ws = 0.0;                         /* No sunrise, continuous nights */
	}
	else if (arg <= -1.0)
	{
		ws = M_PI;                          /* No sunset, continuous days */
	}
	else
	{
		ws = acos(arg);                   /* Sunrise hour angle in radians */
	}
	
	double Eo = 1.00014 - 0.01671 * cos(mnanom) - 0.00014 * cos( 2.0 * mnanom);  // Earth-sun distance (AU)

	solarpos_t *solarpos = malloc(sizeof(solarpos_t));
	assert(solarpos != NULL);
	solarpos->sunrise = 12.0 - ( rad2deg(ws) ) / 15.0 - ( solarpos_inputs->longitude / 15.0 - solarpos_inputs->timezone) - E;
	solarpos->sunset  = 12.0 + ( rad2deg(ws) ) / 15.0 - ( solarpos_inputs->longitude / 15.0 - solarpos_inputs->timezone) - E;
	solarpos->eccentricity = 1.0 / ( Eo * Eo );	// Eccentricity correction factor
	solarpos->true_time = solarpos_inputs->hour + solarpos_inputs->minute / 60.0 + ( solarpos_inputs->longitude / 15.0 - solarpos_inputs->timezone ) + E;	// True solar time (hr) 
	solarpos->azimuth = rad2deg(azm);                        
	solarpos->zenith = rad2deg(0.5 * M_PI - elv);   //  Zenith
	solarpos->elevation = rad2deg(elv);
	solarpos->declination = rad2deg(dec);
	
	return(solarpos);
}

