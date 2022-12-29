#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "csv.h"
#include "parse.h"
#include "debug.h"

typedef enum hdr_type_t {
	HDR_UNKNOWN,
	HDR_RSX,
	HDR_NAV,
	HDR_NAV_RDALT,
	HDR_NAV_LINE,
	HDR_GPS,
	HDR_GPS_GPGGA,
	HDR_GPS_GPZDA,
	HDR_BAR,
	HDR_HUM,
	HDR_TRM,
} hdr_t;

static hdr_t nav_match_header(const char *hdr)
{
	if (hdr == NULL)
		return HDR_UNKNOWN;
		
	if (!strncmp(hdr, "$RDALT", 6))
		return HDR_NAV_RDALT;
	else if (!strncmp(hdr, "$LINE", 5))
		return HDR_NAV_LINE;
	else
		return HDR_UNKNOWN;
}

static hdr_t gps_match_header(const char *hdr)
{
	if (hdr == NULL)
		return HDR_UNKNOWN;
		
	if (!strncmp(hdr, "$GPGGA", 6))
		return HDR_GPS_GPGGA;
	else if (!strncmp(hdr, "$GPZDA", 6))
		return HDR_GPS_GPZDA;
	else
		return HDR_UNKNOWN;
}

static inline int match_header(const char *hdr)
{
	if (hdr == NULL)
		return HDR_UNKNOWN;
	
	if (!strncmp(hdr, "$RSX", 4))
		return HDR_RSX;
	else if (!strncmp(hdr, "$NAV", 4))
		return HDR_NAV;
	else if (!strncmp(hdr, "$GPS", 4))
		return HDR_GPS;
	else if (!strncmp(hdr, "$BAR", 4))
		return HDR_BAR;
	else if (!strncmp(hdr, "$TRM", 4))
		return HDR_TRM;
	else if (!strncmp(hdr, "$HUM", 4))
		return HDR_HUM;								
	else
		return HDR_UNKNOWN;
}

static int data_crc_check(const char *s)
{
	unsigned int crc_calc = 0;
	unsigned int crc_read = 0;
	const char *ptr = NULL;
	
	if (s == NULL)
		return -1;
	
	/* This is a typical gpgga string.
	 * $GPGGA,134259.30,2350.4087,N,07344.9629,E,1,05,4.1,312.48,M,-53.10,M,,*4E
	 * 
	 * Checksum of gps string calculated after '$' till the '*' sign.
	 * The calculated checksum should match with the one embedded in
	 * the gps string for data correctness.
	 */
	for (ptr = s + 1; (*ptr) != '*'; crc_calc ^= *(ptr++))
  		;

	if (sscanf(++ptr, "%2X", &crc_read) != 1) {
		DEBUG("Failed to extract crc bytes from given string.");
		return -1;
	}
	
	if (crc_calc != crc_read) {
		DEBUG("Invalid crc.");
		return -1;
	}
	return 0;
}

static int extract_gps_gpzda_fields(char *str, struct gpzda_fields *zda)
{
	unsigned int hour, min, sec, day, mon, year, tok_nr = 0;
	const char *token = NULL;

	if (data_crc_check(str) != 0) {
		DEBUG("data_crc_check() failed.");
		return -1;
	}
		
	token = strtok(str, ",");
	while ((token = strtok(NULL, ",")) != NULL) {
		switch (++tok_nr) {
		case 1:
			if (sscanf(token, "%2d%2d%2d%*s", &hour, &min, &sec) == 3) {
				zda->utc.tm_hour = hour;
				zda->utc.tm_min = min;
				zda->utc.tm_sec = sec;
			} else {
				DEBUG("Failed to extract hr, min and seconds from gps string.");
			}
			break;

		case 2:
			if (sscanf(token, "%d", &day) == 1) {
				zda->utc.tm_mday = day;
			} else {
				DEBUG("Failed to extract day field from gps string.");
			}
			break;

		case 3:
			if (sscanf(token, "%d", &mon) == 1) {
				zda->utc.tm_mon = mon;
			} else {
				DEBUG("Failed to extract month field from gps string.");
			}
			break;

		case 4:
			if (sscanf(token, "%d", &year) == 1) {
				zda->utc.tm_year = year;
			} else {
				DEBUG("Failed to extract year field from gps string.");
			}
			break;
		}
	}		/* while loop ends */
	return 0;
}

static int extract_gps_gpgga_fields(char *str, struct gpgga_fields *gga)
{
	const char *token = NULL;
	unsigned int tok_nr = 0;

	if (data_crc_check(str) != 0) {
		DEBUG("data_crc_check() failed.");
		return -1;
	}
		
	token = strtok(str, ",");
	while ((token = strtok(NULL, ",")) != NULL) {
		double dv = 0;
		float fv = 0;
		int hr = 0, min = 0, iv = 0;
		unsigned char cv = '?';	
		
		switch (++tok_nr) {
		case 1:
			if (sscanf(token, "%2d%2d%4f%*s", &hr, &min, &fv) == 3) {
				gga->hours = hr;
				gga->minutes = min;
				gga->seconds = fv;
			} else {
				DEBUG("Failed to extract time field from gps string.");
			}
			break;

		case 2:
			if (sscanf(token, "%lf", &dv) == 1) {
				double flr = 0.0;
				dv = dv / 100.0;	/* First two digit are in degrees */
				flr = floor(dv);	/* Get the degree */
				dv = dv - flr;		/* Remaining value gives minutes and seconds */
				/* Two digit of remaining value gives minutes */
				gga->latitude = (100.0 * dv) / 60 + flr;
			} else {
				DEBUG("Failed to extract latitude field from gps string.");
			}
       		break;

		case 3:
			if (sscanf(token, "%c", &cv) == 1) {
				gga->latitude_hemisphere = cv;
			} else {
				DEBUG("Failed to extract latitude hemisphere field from gps string.");
			}
			break;
		
		case 4:
			if (sscanf(token, "%lf", &dv) == 1) {
				double flr = 0.0;			
				dv = dv / 100.0;	/* First two digit are in degrees */
				flr = floor(dv);	/* Get the degree */
				dv = dv - flr;		/* Remaining value gives minutes and seconds */
				/* Two digit of remaining value gives minutes */
				gga->longitude = (100.0 * dv) / 60 + flr;
			} else {
				DEBUG("Failed to extract longitude from gps string.");
			}
			break;

		case 5:
			if (sscanf(token, "%c", &cv) == 1) {
				gga->longitude_hemisphere = cv;
			} else {
				DEBUG("Failed to extract longitude hemisphere field from gps string.");
			}
			break;
			
		case 6:
			if (sscanf(token, "%d", &iv) == 1) {
				if (iv == 1) {
					gga->fix = FIX_GPS;
				} else if (iv == 2) {
					gga->fix = FIX_DGPS;
				} else {
					gga->fix = FIX_INVALID;
				}
			} else {
				DEBUG("Failed to extract fix quality field from gps string.");
			}
			break;

		case 7:
			if (sscanf(token, "%d", &iv) == 1) {
				gga->nsat = iv;	
			} else {
				DEBUG("Failed to extract number of satellites field from gps string.");
			}
			break;

		case 8:
			if (sscanf(token, "%f", &fv) == 1) {
				gga->hdop = fv;
			} else {
				DEBUG("Failed to extract hdop field from gps string.");
			}
			break;
		
		case 9:
			if (sscanf(token, "%f", &fv) == 1) {
				gga->altitude = fv;	
			} else {
				DEBUG("Failed to extract altitude field from gps string.");
			}
			break;

		case 10:
			if (sscanf(token, "%c", &cv) == 1) {
				gga->alt_unit = cv;
			} else {
				DEBUG("Failed to extract altitude unit field from gps string.");
			}
			break;

		case 11:
			if (sscanf(token, "%f", &fv) == 1) {
				gga->geoid_separation = fv;
			} else {
				DEBUG("Failed to extract geoid separation field from gps string.");
			}
			break;

		case 12:
			if (sscanf(token, "%c", &cv) == 1) {
				gga->geoid_separation_unit = cv;
			} else {
				DEBUG("Failed to extract geoid separation unit field from gps string.");
			}	
			break;

		case 13:
			if (sscanf(token, "%d", &iv) == 1) {
				gga->diff_update_age = iv;
			} else {
				DEBUG("Failed to extract diff update age field from gps string.");
			}	
			break;

		case 14:
			if (sscanf(token, "%d", &iv) == 1) {
				gga->base_station_id = iv;
			} else {
				DEBUG("Failed to extract base station id from gps string.");
			}	
			break;
		}
	}
	return 0;
}

static int extract_nav_rdalt_fields(const char *buf, struct rdalt_fields *ral)
{
	double val = 0;	
	if (sscanf(buf, "$RDALT,%lf,", &val) != 1) {
		DEBUG("Failed to extract rdalt field.");
		return -1;
	}
	ral->agl_height = val;
	return 0;
}

static int extract_nav_line_fields(const char *buf, struct line_fields *lin)
{
	unsigned int val = 0;	
	if (sscanf(buf, "$LINE,%d,", &val) != 1) {
		DEBUG("Failed to extract line number field.");
		return -1;
	}
	lin->line_nr = val;
	return 0;
}

static int extract_trm_fields(const char *buf, struct trm_fields *trm)
{
	double val = 0.0;
	
	if (sscanf(buf, "%lf,", &val) != 1) {
		DEBUG("Failed to extract temperature field.");
		return -1;
	}
	
	trm->temperature = val;
	return 0;
}

static int extract_hum_fields(const char *buf, struct hum_fields *hum)
{
	double val = 0.0;
	
	if (sscanf(buf, "%lf,", &val) != 1) {
		DEBUG("Failed to extract humidity field.");
		return -1;
	}
	hum->humidity = val;
	return 0;		
}

static int extract_bar_fields(const char *buf, struct bar_fields *bar)
{
	double val = 0.0;
	
	if (sscanf(buf, "%lf,", &val) != 1) {
		DEBUG("Failed to extract pressure field.");
		return -1;
	}
	bar->pressure = val;
	return 0;		
}

static inline unsigned int two_bytes_to_int(unsigned char hb, unsigned char lb)
{
	return (hb << 8) + lb;
}

static inline unsigned long four_bytes_to_long(unsigned char x, unsigned char y, 
												unsigned char z, unsigned char t)
{
	return ((long)x << 24) + ((long)y << 16) + ((long)z << 8) + (long)t;
}

static int extract_rsx_fields(const unsigned char *buf, struct rsx_fields *rsx)
{
	unsigned char crc = 0;
	const unsigned char *specup, *specdn;
	unsigned int dn_flags, up_flags, err_flags, mask_flags;
	register unsigned int i, j;

	if (memcmp(buf, "\x55\x90\x10\x04", 4) != 0) {
		DEBUG("Invalid GRS data.");
		return -1;
	}
	
	/* crc of header */
	for (i = 0; i < 7; crc ^= buf[i++])
		;
	
	if (crc != buf[7]) {
		DEBUG("crc of header incorrect.");
		return -1;
	}
	
	/* crc of data */
	crc = 0;
	for (i = 8; i < 4246; crc ^= buf[i++])
		;
	if (crc != buf[4247]) {
		DEBUG("crc of data incorrect.");
		return -1;
	}

	/* rsx time */						
	rsx->rsx_time = four_bytes_to_long(buf[15], buf[14], buf[13], buf[12]);

  	dn_flags = two_bytes_to_int(buf[20], buf[19]);
    up_flags = two_bytes_to_int(buf[24], buf[23]);
    err_flags = two_bytes_to_int(buf[28], buf[27]);
    mask_flags = 1;
    
	for (i = 0; i < NR_CRYSTALS; i++) {
       	if (err_flags & mask_flags) {
       		rsx->crystal_labels[i] = 'E';
       		rsx->crystal_error_flags[i] = 1;
       	} else if (dn_flags & mask_flags) {
       		rsx->crystal_labels[i] = 'D';
       		rsx->crystal_error_flags[i] = 0;
       	} else if (up_flags & mask_flags) {
       		rsx->crystal_labels[i] = 'U';
       		rsx->crystal_error_flags[i] = 0;           	
       	} else {
           	rsx->crystal_labels[i] = 'N';
           	rsx->crystal_error_flags[i] = 1;
       	}
       	mask_flags = mask_flags << 1;
	}

	/* extract acquisition time, crystal live time and total gamma counts */    		
	rsx->vd_dn.acq_time = four_bytes_to_long(buf[129], buf[128], buf[127], buf[126]);
   	rsx->vd_up.acq_time = four_bytes_to_long(buf[2190], buf[2189], buf[2188], buf[2187]);
   	rsx->vd_dn.live_time = four_bytes_to_long(buf[133], buf[132], buf[131], buf[130]);
   	rsx->vd_up.live_time = four_bytes_to_long(buf[2194], buf[2193], buf[2192], buf[2191]);
   	rsx->vd_dn.total_gamma_count = two_bytes_to_int(buf[135], buf[134]);
	rsx->vd_up.total_gamma_count = two_bytes_to_int(buf[2196], buf[2195]);

	/* extract up and down spectrum */
	specdn = &buf[138];
	specup = &buf[2199];

	for (i = 0, j = 0; i < NR_CHANNELS; i++, j+= 2) {
		rsx->vd_dn.spectrum[i] = two_bytes_to_int(specdn[j + 1], specdn[j + 0]); 
		rsx->vd_up.spectrum[i] = two_bytes_to_int(specup[j + 1], specup[j + 0]); 
	}
	return 0;	
}

#define NODATA_DURATION		3000

static void warn_on_no_data(double curr_timestamp, double last_timestamp, 
							const char *hdr)
{
	double duration = curr_timestamp - last_timestamp;
	
	if (duration >= NODATA_DURATION) {
		printf("[WARNING at %lf sec]: %s data not found for %.0lf seconds.\n", 
			curr_timestamp, hdr, duration);
	}
}

int parse_dat_file(const char *filename, struct fiducial_data *fid)
{
	FILE *fp = NULL;
	char buf[256] = "";
	double timestamp, prev_timestamp;
	unsigned int init = 1;
	
	if (filename == NULL)
		return -1;
		
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		DEBUG("Failed to open file: %s", filename);
		return -1;
	}

	while (fgets(buf, 256, fp)) {
		hdr_t hdr;
		char *remains = NULL;
		
		/* Checking header */
		hdr = match_header((strtok_r(buf, ",", &remains)));
		if (hdr == HDR_UNKNOWN || !remains)
			continue;
		
		/* Recording time */
		timestamp = atof(strtok_r(remains, ",", &remains));	
		if (remains == NULL)
			continue;
		
		fid->rec_time = floor(timestamp / 1000);
		if (init) {
			prev_timestamp = fid->rec_time;
			fid->rsx.prev_timestamp = fid->rec_time;
			fid->gga.prev_timestamp = fid->rec_time;
			fid->zda.prev_timestamp = fid->rec_time;
			fid->ral.prev_timestamp = fid->rec_time;
			fid->trm.prev_timestamp = fid->rec_time;
			fid->hum.prev_timestamp = fid->rec_time;
			fid->bar.prev_timestamp = fid->rec_time;	
			fid->line.prev_timestamp = fid->rec_time;																				
			init = 0;
		}
		
		if (prev_timestamp >= fid->rec_time) {	
			char rsx_buf[4248] = "";
						
			switch (hdr) {
			case HDR_RSX:
				if (fread(rsx_buf, 4248, 1, fp) == 1) {
					if (!extract_rsx_fields(rsx_buf, &fid->rsx)) {
						fid->rsx.prev_timestamp = fid->rec_time;
					}
				}
				break;
			
			case HDR_GPS:
				switch (gps_match_header(remains)) {
				case HDR_GPS_GPGGA:
					if (!extract_gps_gpgga_fields(remains, &fid->gga)) {
						fid->gga.prev_timestamp = fid->rec_time;
					}					
					break;
				
				case HDR_GPS_GPZDA:
					if (!extract_gps_gpzda_fields(remains, &fid->zda)) {
						fid->zda.prev_timestamp = fid->rec_time;
					}		
					break;
				default:
					break;
				}
		
				break;	
				
			case HDR_TRM:
				if (!extract_trm_fields(remains, &fid->trm)) {
					fid->trm.prev_timestamp = fid->rec_time;
				}
				break;	
				
			case HDR_HUM:
				if (!extract_hum_fields(remains, &fid->hum)) {
					fid->hum.prev_timestamp = fid->rec_time;
				}	
				break;	
		
			case HDR_BAR:
				if (!extract_bar_fields(remains, &fid->bar)) {
					fid->bar.prev_timestamp = fid->rec_time;
				}
				break;
			
			case HDR_NAV:
				switch (nav_match_header(remains)) {
				case HDR_NAV_RDALT:
					if (!extract_nav_rdalt_fields(remains, &fid->ral)) {
						fid->ral.prev_timestamp = fid->rec_time;
					}					
					break;
				
				case HDR_NAV_LINE:
					if (!extract_nav_line_fields(remains, &fid->line)) {
						fid->line.prev_timestamp = fid->rec_time;
					}		
					break;
				}			
		
				break;	
			
			case HDR_UNKNOWN:
		
				break;				
			}
					
		} else {
			prev_timestamp = fid->rec_time;			
			warn_on_no_data(fid->rec_time, fid->trm.prev_timestamp, "Temperature");
			warn_on_no_data(fid->rec_time, fid->hum.prev_timestamp, "Humidity");
			warn_on_no_data(fid->rec_time, fid->bar.prev_timestamp, "Pressure");
			warn_on_no_data(fid->rec_time, fid->rsx.prev_timestamp, "RSX");
			warn_on_no_data(fid->rec_time, fid->gga.prev_timestamp, "GPS GPGGA");
			warn_on_no_data(fid->rec_time, fid->zda.prev_timestamp, "GPS GPZDA");	
			warn_on_no_data(fid->rec_time, fid->ral.prev_timestamp, "NAV RDALT");
			warn_on_no_data(fid->rec_time, fid->line.prev_timestamp, "NAV LINE");					
			csv_format_file(fid);
		}		
	}
	fclose(fp);
	return 0;
}
