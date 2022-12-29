#ifndef CSV_H_INCLUDED
#define CSV_H_INCLUDED

struct fiducial_data;

extern int csv_open_file(const char *filename);
extern void csv_close_file(void);
extern void csv_format_file(const struct fiducial_data *fid);

#endif	/* CSV_H_INCLUDED */
