#ifndef PARSE_H_INCLUDED
#define PARSE_H_INCLUDED

#include <time.h>

#define NR_CHANNELS		1024
#define NR_CRYSTALS		15

struct trm_fields {
	double temperature;
	double prev_timestamp;
};

struct bar_fields {
	double pressure;
	double prev_timestamp;
};

struct hum_fields {
	double humidity;
	double prev_timestamp;
};

struct rdalt_fields {
	double agl_height;
	double prev_timestamp;
};

struct line_fields {
	unsigned int line_nr;
	double prev_timestamp;
};

/* gps fix quality identifier. */
typedef enum gps_fix_t { 
	FIX_INVALID = 0, 
	FIX_GPS, 
	FIX_DGPS 
} gps_fix_t;

/* Structure to keep GPGGA string extracted fields. */
struct gpgga_fields {
	double prev_timestamp;
	double latitude;			/* Latitude value in degrees */
	char latitude_hemisphere;	/* Latitude hemisphere indicator */
	double longitude;			/* Longitude value in degrees */
	char longitude_hemisphere;	/* Longitude hemisphere indicator */
	int hours;
	int minutes;
	float seconds;
	gps_fix_t fix;				/* Quality indicator */
	int	nsat;					/* Number of satellites visible */
	float hdop;					/* horizontal dilution of precision */
	float altitude;				/* Altitude from the MSL */
	char alt_unit;				/* Altitude unit indicator */
	float geoid_separation;		/* diff between WGS84 and MSL */
	char geoid_separation_unit;	/* Unit of geoid separation value */
	int diff_update_age;		/* time since last update */
	int base_station_id;		/* base station id */	
};

struct gpzda_fields {
	double prev_timestamp;
	struct tm utc;
};

struct virtual_detector {
	unsigned long acq_time;
	unsigned long live_time;
	unsigned int total_gamma_count;
	unsigned int spectrum[NR_CHANNELS];
};

struct rsx_fields {
	double prev_timestamp;
	unsigned long rsx_time;
	struct virtual_detector vd_up, vd_dn;
	unsigned char crystal_labels[NR_CRYSTALS];
    unsigned int crystal_error_flags[NR_CRYSTALS];
};

struct fiducial_data {
	double rec_time;
	struct rsx_fields rsx;
	struct trm_fields trm;
	struct hum_fields hum;
	struct bar_fields bar;
	struct rdalt_fields ral;
	struct line_fields line;
	struct gpgga_fields gga;
	struct gpzda_fields zda;
};

extern int parse_dat_file(const char *filename, struct fiducial_data *fid);

#endif	/* PARSE_H_INCLUDED */
