/**
 * @file	tracking_algorithm.h
 * @author	Jason Alderman
 * @date	04SEP2017
 *
 * @brief
 *   Header for tracking algorithm functions
 *
 * @copyright Jason Alderman © 2017 - ALL RIGHTS RESERVED
 */

#ifndef TRACKING_ALGORITHM_H
#define TRACKING_ALGORITHM_H

#include <inttypes.h>
#include "solarpos.h"

typedef struct
{
	double alpha; 		/// Tracker yaw angle in degrees
	double beta; 		/// Tracker pitch angle in degrees
	double gamma; 		/// Tracker roll angle in degrees
	double gcr;			/// Ground coverage ratio fraction
	double night_stow;	/// Night stow angle in degrees
	double rom;			/// Range of motion in degrees
} tracker_t;

double tracker_incident(tracker_t *tracker, solarpos_t *solarpos);
double tracker_angle(solarpos_t *solarpos, tracker_t *tracker);
double shade_avoidance_angle(double tracker_angle, tracker_t *tracker);

#endif
