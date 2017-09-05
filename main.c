/*
 * Calculate ideal tracker angle (without any shade avoidance) every minute over the course of 1 year
 * and write results for each location to a CSV file.  A summary file is also created.
 *
 * Finally, we answer the question: "what percent of the time does a tracker spend at +/- 5 degrees?"
 * 
 * Jason Alderman
 * 04SEP2017
 * jason.alderman@hotmail.com
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include "tracking_algorithm.h"
#include "angle_conversions.h"
#include "solarpos.h"

typedef struct
{
	const double latitude;
	const double longitude;
	const int8_t timezone;
	const char *name;
	double percent_in_zone; // percent of time within the +/- 5 degree range of interest
} location_t;

typedef struct
{
	double angle_bin;
	uint32_t count;
} location_summary_t;

/****************************************************************************/
// Global variables
#define TRACKER_ROM		60 		// degrees
#define TRACKER_GCR		0.35 	// ground coverage ratio fraction
#define TRACKER_STOW	-10 	// night stow angle in degrees

#define ANGLE_BIN_SIZE	5.0		// degrees

#define NUM_LOCATIONS	5
static location_t locations[NUM_LOCATIONS] = {
	{47.608358, -122.323175, -8, "Seattle",       0}, 
	{37.768977, -122.440647, -8, "San_Francisco", 0},
	{19.435303, -99.1438270, -6, "Mexico_City",   0},
	{32.728205, -117.137621, -8, "San_Diego",     0},
	{61.160612, -150.014821, -9, "Anchorage",     0}
};

tracker_t tracker;
uint16_t year = 2017;
int8_t month_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/****************************************************************************/


int main(int argc, char* argv[])
{
	uint8_t month, day, hour, minute;
	tracker.rom = TRACKER_ROM;
	tracker.gcr = TRACKER_GCR;
	tracker.night_stow = TRACKER_STOW;
	tracker.alpha = 0;
	tracker.beta = 0;
	
	FILE *summary_file = fopen("AngleSummary_All.csv", "w");
	if (summary_file == NULL)
	{
		printf("Error opening summary file!\n");
		exit(1);
	}
	fprintf(summary_file, "LOCATION,ANGLE_BIN,COUNT,PERCENT_OF_TIME\n");

	uint8_t i;
	for (i=0; i<NUM_LOCATIONS; i++)
	{
		printf("Calculating data for %s\n", locations[i].name);
		
		// Open raw data file where we will save all angle data for the year
		char fname[32] = {0};
		strcat(fname, "TrackerAngle_");
		strcat(fname, locations[i].name);
		strcat(fname, ".csv");
		FILE *location_file = fopen(fname, "w");
		if (location_file == NULL)
		{
			printf("Error opening output file for %s \n", locations[i].name);
			exit(1);
		}
		fprintf(location_file, "LOCATION,YEAR,MONTH,DAY,HOUR,MINUTE,ANGLE\n");
		
		
		// Initialize summary struct
		uint32_t num_bins = (uint32_t)(TRACKER_ROM/ANGLE_BIN_SIZE) + 1;
		location_summary_t location_summary[num_bins];
		uint32_t j;
		for (j=0; j<num_bins; j++)
		{
			location_summary[j].angle_bin = j*ANGLE_BIN_SIZE;
			location_summary[j].count = 0;
		}
		
		for (month=0; month<12; month++)
		{
			
			for (day=1; day<=month_days[month]; day++)
			{
				
				for (hour=0; hour<24; hour++)
				{
					
					for(minute=0; minute<60; minute++)
					{
						solarpos_inputs_t solarpos_inputs;
						solarpos_inputs.year 		= year;
						solarpos_inputs.month 		= month+1; // algorith expects calendar month
						solarpos_inputs.day 		= day;
						solarpos_inputs.hour 		= hour;
						solarpos_inputs.minute 		= minute;
						solarpos_inputs.latitude	= locations[i].latitude;
						solarpos_inputs.longitude	= locations[i].longitude;
						solarpos_inputs.timezone	= locations[i].timezone;
						
						// Calculate solar position and tracker angle for this location at this time
						solarpos_t *solarpos = solar_position_calc(&solarpos_inputs);
						//~ printf("Month %02d Day %02d Hour %02d Minute %02d Az %.3f El %.3f - ",
							//~ month+1, day, hour, minute, solarpos->azimuth, solarpos->elevation);
							
						double angle_no_sa = tracker_angle(solarpos, &tracker);
						double angle_w_sa = shade_avoidance_angle(angle_no_sa, &tracker);
						//~ printf("w/o SA %.1f, w/SA %.1f\n", angle_no_sa, angle_w_sa);
						
						// Save to raw data file
						fprintf(location_file, "%s,%02d,%02d,%02d,%02d,%02d,%.1f\n",
							locations[i].name, year, month, day, hour, minute, angle_w_sa);
							
						// Update location summary struct
						uint16_t bin = (uint16_t)(abs(angle_w_sa) / ANGLE_BIN_SIZE);
						location_summary[bin].count++;
					}
				}
			}
		}
		
		fclose(location_file);
		
		// Write angle summary file
		memset(fname, 0, 32);
		strcat(fname, "AngleSummary_");
		strcat(fname, locations[i].name);
		strcat(fname, ".csv");
		FILE *location_summary_file = fopen(fname, "w");
		if (location_summary_file == NULL)
		{
			printf("Error opening output file for %s \n", locations[i].name);
			exit(1);
		}
		fprintf(location_summary_file, "LOCATION,ANGLE_BIN,COUNT,PERCENT_OF_TIME\n");
		for (j=0; j<num_bins; j++)
		{
			fprintf(location_summary_file, "%s,%.1f,%d,%.3f\n",
				locations[i].name, location_summary[j].angle_bin, location_summary[j].count, (100 * (double)location_summary[j].count/(365*24*60)) );
				
			fprintf(summary_file, "%s,%.1f,%d,%.3f\n",
				locations[i].name, location_summary[j].angle_bin, location_summary[j].count, (100 * (double)location_summary[j].count/(365*24*60)) );
		}
		fclose(location_summary_file);
		
		// Calculate percent of time this location's tracker is within the range of interest (+/- 5 deg)
		double count_in_zone = 0;
		j = 0;
		while (j*ANGLE_BIN_SIZE < 5)
		{
			count_in_zone += location_summary[j].count;
			j++;
		}
		locations[i].percent_in_zone = 100.0 * (double)count_in_zone / (365*24*60);
	}
	
	fclose(summary_file);
	
	// Print a table showing percent of time at +/- 5 degrees for each location
	printf("\nLocation           %% in Zone\n");
	for (i=0; i<NUM_LOCATIONS; i++)
	{
		printf("%-18s %.2f\n", locations[i].name, locations[i].percent_in_zone);
	}
	
	exit(0);
}
