/**
 * @file	tracking_algorithm.c
 * @author	Jason Alderman
 * @date	04SEP2017
 *
 * @brief
 *   Single axis solar tracking algorithm functions
 *
 * @copyright Jason Alderman © 2017 - ALL RIGHTS RESERVED
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "tracking_algorithm.h"
#include "angle_conversions.h"

/**
 * @brief
 *  Find angle of incidence for single axis tracker with the specified roll, pitch and yaw angles
 * 
 * @param [in] tracker pointer to tracker_t struct
 * @param [in] solarpos pointer to solarpos_t struct
 * 
 * @return ia Angle of incidence in degrees between sun and tracker 
*/
double tracker_incident(tracker_t *tracker, solarpos_t *solarpos)
{
	// convert angles to radians 
	double gamma = deg2rad(tracker->gamma);
	double beta = deg2rad(tracker->beta);
	double alpha = deg2rad(tracker->alpha);
	double theta = deg2rad(360 - solarpos->azimuth);
	double phi = deg2rad(solarpos->zenith);

	// get tracker normal vector in cartesian coordinates
	double tracker_x = cos( gamma) * sin(beta) * cos(alpha) +  -sin(gamma) * -sin(alpha);
	double tracker_y = cos( gamma) * sin(beta) * sin(alpha) +  -sin(gamma) * cos(alpha);
	double tracker_z = cos( gamma ) * cos(beta);
	
	// sun vector in cartesian coordinates
	double sun_x = cos(theta) * sin(phi);
	double sun_y = sin(theta) * sin(phi);
	double sun_z = cos(phi);

	// AOI is acos of dot product of sun and tracker
	double ia = acos(sun_x * tracker_x + sun_y * tracker_y + sun_z * tracker_z);
	
	return rad2deg(ia);
}







/**
 * @brief
 *  Find 3DOF single axis tracker angle, Jason's method.
 * 
 * @param [in] solarpos pointer to solarpos_t struct
 * @param [in] tracker pointer to tracker_t struct
 * 
 * @return Ideal tracker angle in degrees without shade avoidance
 */

double tracker_angle(solarpos_t *solarpos, tracker_t *tracker) 
{
	// return stow angle when sun is below the horizon
	if (solarpos->zenith >= 90.0) 
	{
		return(tracker->night_stow); 
	}
	
	// convert angles to radians
	double beta = deg2rad(tracker->beta);
	double alpha = deg2rad(tracker->alpha);
	double theta = deg2rad(360 - solarpos->azimuth);
	double phi = deg2rad(solarpos->zenith);

	// this is a bit long, so I've split it up
	double A = cos(alpha) * sin(phi) * sin(theta) - sin(alpha) * sin(phi) * cos(theta);
	double B = sin(alpha) * sin(beta) * sin(phi) * sin(theta) + cos(alpha) * sin(beta) * sin(phi) * cos(theta) + cos(beta) * cos(phi);
	double calculated_angle = -atan(A / B);
	
	// we need to change the sign of the tracker angle when 
	// the sun azimuth angle is not between -pi/2 and pi/2
	double q = atan2(A, B);
	if (q > M_PI/2 
		|| 
		q < (-1 * M_PI)/2) 
	{
		calculated_angle = -calculated_angle;
	}
	
	tracker->gamma = rad2deg(calculated_angle);
	return(rad2deg(calculated_angle));
}






/**
 * @brief
 *  Tracker shade avoidance function for tilted single axis trackers (tilt can be zero).
 *  Calculates the angle to which the panels must backdrive to avoid shading (returns tracker_angle when no shading would occur).
 *
 * @param [in] tracker_angle Ideal tracker angle in degrees without shade avoidance
 * @param [in] tracker pointer to tracker_t struct
 * 
 * @return Tracker angle in degrees WITH shade avoidance taken into account.
*/
double shade_avoidance_angle(double tracker_angle, tracker_t *tracker) 
{
	double angle_sa;
	double direct_cutoff = rad2deg(acos(tracker->gcr)); // This is the angle at which backtracking begins

	if ( abs(tracker_angle) <= direct_cutoff ) 
	{
		angle_sa = tracker_angle;
	} 
	else 
	{
		double gamma = 90.0 - tracker_angle;

		if (tracker_angle < 0) 
		{
			double beta1 = asin( sin(deg2rad(gamma)) / tracker->gcr );
			angle_sa = deg2rad(180) - beta1 - deg2rad(gamma); // angle in radians
		} 
		else 
		{
			double beta2 = deg2rad(180) - asin( sin(deg2rad(gamma)) / tracker->gcr );
			angle_sa = deg2rad(180) - beta2 - deg2rad(gamma); // angle in radians
		}
		angle_sa = rad2deg(angle_sa);
	}
	
	// keep angle within range of motion
	if (angle_sa < -tracker->rom) 
	{
		return(-tracker->rom);
	}
	if (angle_sa > tracker->rom) 
	{
		return(tracker->rom);
	}

	return(angle_sa);
}




