/************************************************************************

 File:				sprat_red_trace.c
 Last Modified Date:     	30/09/14

************************************************************************/

#include <string.h>
#include <stdio.h>
#include "fitsio.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sprat_error_handling.h"
#include "sprat_functions.h"
#include "sprat_config.h"
#include "sprat_red_trace.h"
#include "sprat_red_find.h"

// *********************************************************************

int main(int argc, char *argv []) {
	if(populate_env_variable(REF_ERROR_CODES_FILE, "L2_ERROR_CODES_FILE")) {

		printf("\nUnable to populate [REF_ERROR_CODES_FILE] variable with corresponding environment variable. Routine will proceed without error handling\n");

	}

	if (argc != 2) {

		if(populate_env_variable(SPR_BLURB_FILE, "L2_SPR_BLURB_FILE")) {

			RETURN_FLAG = 1;

		} else {

			print_file(SPR_BLURB_FILE);

		}

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", -1, "Status flag for L2 sptrace routine", ERROR_CODES_FILE_WRITE_ACCESS);

		return 1;

	} else {
		// ***********************************************************************
		// Redefine routine input parameters

		int order	= strtol(argv[1], NULL, 0);	

		// ***********************************************************************
		// Open [SPFIND_OUTPUTF_PEAKS_FILE] input file
	
		FILE *inputfile;
	
		if (!check_file_exists(SPFIND_OUTPUTF_PEAKS_FILE)) { 

			inputfile = fopen(SPFIND_OUTPUTF_PEAKS_FILE , "r");

		} else {

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", -2, "Status flag for L2 sptrace routine", ERROR_CODES_FILE_WRITE_ACCESS);

			return 1;

		}

		// ***********************************************************************
		// Find some [SPFIND_OUTPUTF_PEAKS_FILE] input file details

		char input_string [150];
		
		int row_count = 0;
		while(!feof(inputfile)) {
			memset(input_string, '\0', sizeof(char)*150);
			fgets(input_string, 150, inputfile);
			if (strtol(&input_string[0], NULL, 0) > 0) {		// check the line begins with a positive number (usable)
				row_count++;
			}
		}
		
		rewind(inputfile);
		
		// ***********************************************************************
		// Store [SPFIND_OUTPUTF_PEAKS_FILE] data		
		
		double x_coords[row_count];
		memset(x_coords, 0, sizeof(double)*(row_count));

		double y_coords[row_count];
		memset(y_coords, 0, sizeof(double)*(row_count));

		double coord_x, coord_y;
		int idx = 0;
		while(!feof(inputfile)) {
			memset(input_string, '\0', sizeof(char)*150);
			fgets(input_string, 150, inputfile);	
			if (strtol(&input_string[0], NULL, 0) > 0) {		// check the line begins with a positive number (usable)
				sscanf(input_string, "%lf\t%lf\n", &coord_x, &coord_y);
				x_coords[idx] = coord_x;
				y_coords[idx] = coord_y;
				idx++;
			}
		}
		
		// ***********************************************************************
		// Perform a few checks to ensure the input tracing parameters 
		// are sensible

		if ((order < SPTRACE_VAR_POLYORDER_LO) || (order > SPTRACE_VAR_POLYORDER_HI)) {	// Check [order] is within config limits
			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", -3, "Status flag for L2 sptrace routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fclose(inputfile);
			return 1; 
		}
		
		// ***********************************************************************
		// Create [SPTRACE_OUTPUTF_TRACES_FILE] output file and print a few 
		// parameters

		FILE *outputfile;
		outputfile = fopen(SPTRACE_OUTPUTF_TRACES_FILE, FILE_WRITE_ACCESS);

		if (!outputfile) { 
			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", -4, "Status flag for L2 sptrace routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fclose(inputfile);
			return 1;
		}

		char timestr [80];
		memset(timestr, '\0', sizeof(char)*80);

		find_time(timestr);

		fprintf(outputfile, "#### %s ####\n\n", SPTRACE_OUTPUTF_TRACES_FILE);
		fprintf(outputfile, "# Lists the trace coefficients and corresponding chi-squareds found using the sptrace program.\n\n");
		fprintf(outputfile, "# Run datetime:\t\t%s\n", timestr);
		fprintf(outputfile, "# Polynomial Order:\t%d\n\n", order);
		
		// ***********************************************************************
		// Fit and store results to [SPTRACE_OUTPUTF_TRACES_FILE] file

		double coeffs[order];
		double this_chi_squared;
		if (calc_least_sq_fit(order, row_count, x_coords, y_coords, coeffs, &this_chi_squared)) {	// reversed [coord_y] and [coord_x] as want to find x = f(y) not y = f(x)
			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", -5, "Status flag for L2 frtrace routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fclose(inputfile);
			fclose(outputfile);
			return 1; 
		}	

		int ii;
		for (ii=0; ii<=order; ii++) {
			fprintf(outputfile, SPTRACE_VAR_ACCURACY_COEFFS, coeffs[ii]);
			fprintf(outputfile, "\t");
		}
		fprintf(outputfile, SPTRACE_VAR_ACCURACY_CHISQ, this_chi_squared);
		fprintf(outputfile, "\n");
		fprintf(outputfile, "%d", EOF);

		printf("\nFitting results");
		printf("\n--------------------\n");
		printf("\nχ2:\t\t\t%.2f\n", this_chi_squared);
		
		// ***********************************************************************
		// Perform a few checks to ensure the chi squareds are sensible 

		if ((this_chi_squared < SPTRACE_VAR_CHISQUARED_MIN) || (this_chi_squared > SPTRACE_VAR_CHISQUARED_MAX)) {	// comparing doubles but accuracy isn't a necessity so don't need gsl_fcmp function
			RETURN_FLAG = 2;
		}		
		
		// ***********************************************************************
		// Close [SPFIND_OUTPUTF_PEAKS_FILE] input file and 
		// [SPTRACE_OUTPUTF_TRACES_FILE] output file

		if (fclose(inputfile)) {
			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", 999/*TODO*/, "Status flag for L2 sptrace routine", ERROR_CODES_FILE_WRITE_ACCESS);
			return 1; 
		}
		
		if (fclose(outputfile)) {
			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", 999/*TODO*/, "Status flag for L2 sptrace routine", ERROR_CODES_FILE_WRITE_ACCESS);
			return 1; 
		}		
		
		// Write success to [ERROR_CODES_FILE]

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTC", RETURN_FLAG, "Status flag for L2 sptrace routine", ERROR_CODES_FILE_WRITE_ACCESS);

		return 0;

	}

}

