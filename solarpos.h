/**
 * @file	solarpos.h
 * @author	Jason Alderman
 * @date	04SEP2017
 *
 * @brief
 *   Header for solar position algorithm
 *
 * @copyright Jason Alderman © 2017 - ALL RIGHTS RESERVED
 */

#ifndef SOLARPOS_H
#define SOLARPOS_H

/// elements to return sun parameters to calling function
typedef struct {
	double azimuth; 			/// sun azimuth in degrees, measured east from north
	double zenith; 				/// sun zenith in degrees
	double elevation; 			/// sun elevation in degrees
	double declination; 		/// sun declination in degrees
	double sunrise; 			/// sunrise in local standard time (hrs), not corrected for refraction
	double sunset; 				/// sunset in local standard time (hrs), not corrected for refraction
	double eccentricity; 		/// Eo, eccentricity correction factor
	double true_time; 			/// true solar time (hrs) 
} solarpos_t;

typedef struct {
	uint16_t year;				/// Year, e.g. 2017
	uint8_t  month; 			/// Calendar month of year (e.g. 1=Jan)
	uint8_t  day;				/// Day of calendar month (1-31)
	uint8_t  hour;				/// Hour in localtime, 0-23
	uint8_t  minute;			/// minutes past the hour, local standard time
	int8_t   timezone;			/// time zone, west longitudes negative

	double latitude;			/// Decimal latitude
	double longitude;			/// Decimal longitude
} solarpos_inputs_t;

solarpos_t *solar_position_calc(solarpos_inputs_t *solarpos_inputs);

#endif

