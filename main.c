#include <stdio.h>
#include <string.h>

#include "csv.h"
#include "parse.h"
#include "debug.h"

int main(int argc, char **argv) 
{
	register int i;
	struct fiducial_data fid;
	
	memset(&fid, 0, sizeof(fid));
	
	csv_open_file("tmp.csv");
	
	for (i = 1; i < argc; i++) {
		DEBUG("Extracting file: %s", argv[i]);
    	parse_dat_file(argv[i], &fid); 
	}
	csv_close_file();
	return 0;
}

