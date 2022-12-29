#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include "csv.h"
#include "debug.h"
#include "parse.h"

static FILE *fp_csv = NULL;

static void format_data(FILE *fp, const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	vfprintf(fp, fmt, ap);
	va_end(ap);
}

static void format_header(void)
{
	register unsigned int i;
	
	format_data(fp_csv, "%s", "REC_TIME,GPS_DATE,GPS_TIME,GPS_LAT,GPS_LON,"
							"GPS_ALT,GPS_FIX,GPS_SATS,GPS_HDOP,RAD_ALT,"
							"LINE_NUM,BAR,TRM,HUM,MAG,MAG_AMP,RSX_TIME,");
	
	for (i = 1; i < NR_CRYSTALS + 1; i++)
		format_data(fp_csv, "CR%02d,", i);
	for (i = 1; i < NR_CRYSTALS + 1; i++)
		format_data(fp_csv, "CR_ERR%02d,", i);

	format_data(fp_csv, "%s", "ACQ_TIME_D,ACQ_TIME_U,LIVE_TIME_D,"
							"LIVE_TIME_U,GAMMA_TOTAL_D,GAMMA_TOTAL_U,");

	
	//for (i = 1; i < NR_CHANNELS + 1; i++)
		//format_data(fp_csv, "D%04d,", i);
	//for (i = 1; i < NR_CHANNELS + 1; i++)
		//format_data(fp_csv, "U%04d,", i);	

	format_data(fp_csv, "%s", "\n");	
}

int csv_open_file(const char *filename)
{
	if (filename == NULL)
		return -1;
		
	fp_csv = fopen(filename, "w");
	if (fp_csv == NULL) {
		DEBUG("Failed to open file: %s", filename);
		return -1;
	}
	format_header();
	return 0;
}


void csv_format_file(const struct fiducial_data *fid)
{
	register unsigned int i;
	
	format_data(fp_csv, "%lf,", fid->rec_time);
	format_data(fp_csv, "%04d/%02d/%02d,", fid->zda.utc.tm_year, 
						fid->zda.utc.tm_mon, fid->zda.utc.tm_mday);	
	format_data(fp_csv, "%02d:%02d:%04.2f,", fid->gga.hours,
						fid->gga.minutes, fid->gga.seconds);
	format_data(fp_csv, "%7.4lf,", fid->gga.latitude);
	format_data(fp_csv, "%7.4lf,", fid->gga.longitude);			
	format_data(fp_csv, "%.2f,", fid->gga.altitude);
	format_data(fp_csv, "%i,", fid->gga.fix);
	format_data(fp_csv, "%d,", fid->gga.nsat);
	format_data(fp_csv, "%.1f,", fid->gga.hdop);					
	format_data(fp_csv, "%.1f,", fid->ral.agl_height);
	format_data(fp_csv, "%d,", fid->line.line_nr);
	format_data(fp_csv, "%.2lf,", fid->bar.pressure);
	format_data(fp_csv, "%.2lf,%.2lf,", fid->trm.temperature, fid->hum.humidity);		
	format_data(fp_csv, ",,");
	format_data(fp_csv, "%ld,", fid->rsx.rsx_time);		
    		
	for (i = 0; i < NR_CRYSTALS; i++)
		format_data(fp_csv, "%c,", fid->rsx.crystal_labels[i]);
    		
	for (i = 0; i < NR_CRYSTALS; i++)
		format_data(fp_csv, "%d,", fid->rsx.crystal_error_flags[i]);

	format_data(fp_csv, "%ld,", fid->rsx.vd_dn.acq_time);
	format_data(fp_csv, "%ld,", fid->rsx.vd_up.acq_time);			
	format_data(fp_csv, "%ld,", fid->rsx.vd_dn.live_time);
	format_data(fp_csv, "%ld,", fid->rsx.vd_up.live_time);			
	format_data(fp_csv, "%d,", fid->rsx.vd_dn.total_gamma_count);
	format_data(fp_csv, "%d,", fid->rsx.vd_up.total_gamma_count);
	
	//for (i = 0; i < NR_CHANNELS; i++)
		//format_data(fp_csv, "%d,", fid->rsx.vd_dn.spectrum[i]);
	//for (i = 0; i < NR_CHANNELS; i++)
		//format_data(fp_csv, "%d,", fid->rsx.vd_up.spectrum[i]);
	
	format_data(fp_csv, "\n");												
}

void csv_close_file(void)
{
	fclose(fp_csv);
	fp_csv = NULL;
}
