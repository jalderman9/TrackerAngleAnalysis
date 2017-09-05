/**
 * @file	angle_conversions.h
 * @author	Jason Alderman
 * @date	04SEP2017
 *
 * @brief
 *   Functions to convert between radians and degrees
 *
 * @copyright Jason Alderman © 2017 - ALL RIGHTS RESERVED
 */

#ifndef ANGLE_CONVERSIONS_H
#define ANGLE_CONVERSIONS_H

// Some versions of math.h already define M_PI
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795028841971
#endif

 
#define DEG_CIRCLE 360
#define DEG_TO_RAD (M_PI / (DEG_CIRCLE / 2))
#define RAD_TO_DEG ((DEG_CIRCLE / 2) / M_PI)

static inline double deg2rad(double degrees) 
{
    return(degrees * DEG_TO_RAD);
}

 
static inline double rad2deg(double radians) 
{
    return(radians * RAD_TO_DEG);
} 

#endif
