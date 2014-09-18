/************************************************************************

 File:				sprat_red_findedges.c
 Last Modified Date:     	11/09/14

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
#include "sprat_red_trim.h"

#include <gsl/gsl_sort_double.h>
#include <gsl/gsl_statistics_double.h>

// *********************************************************************

int main(int argc, char *argv []) {

	if(populate_env_variable(REF_ERROR_CODES_FILE, "L2_ERROR_CODES_FILE")) {

		printf("\nUnable to populate [REF_ERROR_CODES_FILE] variable with corresponding environment variable. Routine will proceed without error handling\n");

	}

	if (argc != 9) {

		if(populate_env_variable(SPT_BLURB_FILE, "L2_SPT_BLURB_FILE")) {

			RETURN_FLAG = 1;

		} else {

			print_file(SPT_BLURB_FILE);

		}

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -1, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

		return 1;

	} else {

		char time_start [80];
		memset(time_start, '\0', sizeof(char)*80);

		find_time(time_start);

		write_additional_key_to_file_str(ADDITIONAL_KEYS_FILE, "STARTDATE", "L2DATE", time_start, "when this reduction was performed", ADDITIONAL_KEYS_FILE_WRITE_ACCESS);

		// ***********************************************************************
		// Redefine routine input parameters

		char *cont_f			= strdup(argv[1]);
		char *in_f			= strdup(argv[2]);		
		int bin_size			= strtol(argv[3], NULL, 0);
		double bg_level_quantile	= strtod(argv[4], NULL);
		int scan_window_size_px		= strtol(argv[5], NULL, 0);
		int scan_window_trig_nsigma	= strtol(argv[6], NULL, 0);
		int min_spectrum_width		= strtol(argv[7], NULL, 0);
		char *out_f			= strdup(argv[8]);
		
		// ***********************************************************************
		// Open cont file (ARG 1), get parameters and perform any data format 
		// checks

		fitsfile *cont_f_ptr;

		int cont_f_maxdim = 2, cont_f_status = 0, cont_f_bitpix, cont_f_naxis;
		long cont_f_naxes [2] = {1,1};

		if(!fits_open_file(&cont_f_ptr, cont_f, IMG_READ_ACCURACY, &cont_f_status)) {

			if(!populate_img_parameters(cont_f, cont_f_ptr, cont_f_maxdim, &cont_f_bitpix, &cont_f_naxis, cont_f_naxes, &cont_f_status, "CONTINUUM FRAME")) {

				if (cont_f_naxis != 2) {	// any data format checks here

					write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -2, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

					free(cont_f);
					free(in_f);					
					free(out_f);
					if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 

					return 1;
	
				}

			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -3, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
				fits_report_error(stdout, cont_f_status); 

				free(cont_f);
				free(in_f);					
				free(out_f);
				if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 

				return 1; 

			}

		} else { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -4, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
			fits_report_error(stdout, cont_f_status); 

			free(cont_f);
			free(in_f);					
			free(out_f);
			
			return 1; 

		}
		
		// ***********************************************************************
		// Open input file (ARG 2), get parameters and perform any data format 
		// checks

		fitsfile *in_f_ptr;

		int in_f_maxdim = 2, in_f_status = 0, in_f_bitpix, in_f_naxis;
		long in_f_naxes [2] = {1,1};

		if(!fits_open_file(&in_f_ptr, in_f, IMG_READ_ACCURACY, &in_f_status)) {

			if(!populate_img_parameters(in_f, in_f_ptr, in_f_maxdim, &in_f_bitpix, &in_f_naxis, in_f_naxes, &in_f_status, "INPUT FRAME")) {

				if (in_f_naxis != 2) {	// any data format checks here

					write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -5, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

					free(cont_f);
					free(in_f);					
					free(out_f);
					if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
					if(fits_close_file(in_f_ptr, &in_f_status)) fits_report_error (stdout, in_f_status);					

					return 1;
	
				}

			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -6, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
				fits_report_error(stdout, in_f_status); 

				free(cont_f);
				free(in_f);					
				free(out_f);
				if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
				if(fits_close_file(in_f_ptr, &in_f_status)) fits_report_error (stdout, in_f_status);

				return 1; 

			}

		} else { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -7, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
			fits_report_error(stdout, in_f_status); 

			free(cont_f);
			free(in_f);					
			free(out_f);
			
			return 1; 

		}	
		
		// ***********************************************************************
		// Check consistency of arc/target/continuum fits files (ARGS 1, 2 and 3)
	
		printf("\nConsistency check");
		printf("\n-----------------\n");

		printf("\nBits per pixel:\t\t");

		if (cont_f_bitpix != in_f_bitpix) { 	// if a = b and b = c then a must = c

			printf("FAIL\n"); 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATAR", -8, "Status flag for L2 sptrim routine", ERROR_CODES_FILE_WRITE_ACCESS);

			free(cont_f);
			free(in_f);					
			free(out_f);
			
			if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
			if(fits_close_file(in_f_ptr, &in_f_status)) fits_report_error (stdout, in_f_status);

			return 1; 

		} else { 

			printf("OK\n"); 

		} 

		printf("Number of axes:\t\t");

		if (cont_f_naxis != in_f_naxis) {	
	
			printf("FAIL\n"); 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATAR", -9, "Status flag for L2 sptrim routine", ERROR_CODES_FILE_WRITE_ACCESS);

			free(cont_f);
			free(in_f);					
			free(out_f);
			
			if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
			if(fits_close_file(in_f_ptr, &in_f_status)) fits_report_error (stdout, in_f_status);
			
			return 1; 

		} else { 

			printf("OK\n"); 

		} 
	
		printf("First axis dimension:\t");

		if (cont_f_naxes[0] != in_f_naxes[0]) {	
	
			printf("FAIL\n"); 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATAR", -10, "Status flag for L2 sptrim routine", ERROR_CODES_FILE_WRITE_ACCESS);

			free(cont_f);
			free(in_f);					
			free(out_f);
			
			if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
			if(fits_close_file(in_f_ptr, &in_f_status)) fits_report_error (stdout, in_f_status);

			return 1; 

		} else { 

			printf("OK\n"); 

		} 

		printf("Second axis dimension:\t");

		if (cont_f_naxes[1] != in_f_naxes[1]) {

			printf("FAIL\n"); 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATAR", -11, "Status flag for L2 sptrim routinee", ERROR_CODES_FILE_WRITE_ACCESS);

			free(cont_f);
			free(in_f);					
			free(out_f);
			
			if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
			if(fits_close_file(in_f_ptr, &in_f_status)) fits_report_error (stdout, in_f_status);
			
			return 1; 

		} else { 

			printf("OK\n");
			

		}				

		// ***********************************************************************
		// Set the range limits n.b. this should be an arbitrary choice if all 
		// files have identical parameters

		int cut_x [2] = {1, cont_f_naxes[0]};
		int cut_y [2] = {1, cont_f_naxes[1]};

		// ***********************************************************************
		// Set parameters used when reading data from continuum and input fits 
		// file (ARGS 1 and 2)

		long fpixel [2] = {cut_x[0], cut_y[0]};
		long nxelements = (cut_x[1] - cut_x[0]) + 1;
		long nyelements = (cut_y[1] - cut_y[0]) + 1;

		// ***********************************************************************
		// Create arrays to store pixel values from continuum and input fits file 
		// (ARGS 1 and 2)

		double cont_f_pixels [nxelements];
		double in_f_pixels [nxelements];

		// ***********************************************************************
		// Get continuum fits file (ARG 1) values and store in 2D array

		int ii;

		double cont_frame_values [nyelements][nxelements];
		memset(cont_frame_values, 0, sizeof(double)*nxelements*nyelements);

		for (fpixel[1] = cut_y[0]; fpixel[1] <= cut_y[1]; fpixel[1]++) {

			memset(cont_f_pixels, 0, sizeof(double)*nxelements);

			if(!fits_read_pix(cont_f_ptr, TDOUBLE, fpixel, nxelements, NULL, cont_f_pixels, NULL, &cont_f_status)) {

				for (ii=0; ii<nxelements; ii++) {

					cont_frame_values[fpixel[1]-1][ii] = cont_f_pixels[ii];

				}

			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -12, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
				fits_report_error(stdout, cont_f_status); 

				free(cont_f);
				free(in_f);					
				free(out_f);				
				if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 

				return 1; 

			}

		}
		
		// ***********************************************************************
		// Get input fits file (ARG 2) values and store in 2D array

		double in_frame_values [nyelements][nxelements];
		memset(in_frame_values, 0, sizeof(double)*nxelements*nyelements);

		for (fpixel[1] = cut_y[0]; fpixel[1] <= cut_y[1]; fpixel[1]++) {

			memset(in_f_pixels, 0, sizeof(double)*nxelements);

			if(!fits_read_pix(in_f_ptr, TDOUBLE, fpixel, nxelements, NULL, in_f_pixels, NULL, &in_f_status)) {

				for (ii=0; ii<nxelements; ii++) {

					in_frame_values[fpixel[1]-1][ii] = in_f_pixels[ii];

				}

			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -13, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
				fits_report_error(stdout, cont_f_status); 

				free(cont_f);
				free(in_f);					
				free(out_f);				
				if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 

				return 1; 

			}

		}
				
		
		// BIN ARRAY AND FIND BACKGROUND
		// ***********************************************************************		
		// 1.	Bin array according to bin width given by [bin_size]
			
		int disp_nelements, spat_nelements;
		disp_nelements = nxelements;
		spat_nelements = nyelements;
		

		int disp_nelements_binned = (int)floor(disp_nelements/bin_size);			
		double this_frame_values_binned[spat_nelements][disp_nelements_binned];
		memset(this_frame_values_binned, 0, sizeof(double)*spat_nelements*disp_nelements_binned);
		
		int jj;
		double this_bin_values;
		int bin_number = 0;
		for (jj=0; jj<spat_nelements; jj++) {
			this_bin_values = 0;
			bin_number = 0;
			for (ii=0; ii<disp_nelements; ii++) {
				if (ii % bin_size == 0 && ii != 0) {
					this_frame_values_binned[jj][bin_number] = this_bin_values;
					bin_number++;
					this_bin_values = 0;
				}
				this_bin_values += cont_frame_values[jj][ii];
			}
		}	
		
		// 2.	Transform to 1D array
		double this_frame_values_binned_1D[spat_nelements*disp_nelements_binned];
		memset(this_frame_values_binned_1D, 0, sizeof(double)*spat_nelements*disp_nelements_binned);
		
		int idx = 0;
		for (jj=0; jj<spat_nelements; jj++) {
			for (ii=0; ii<disp_nelements_binned; ii++) {
				this_frame_values_binned_1D[idx] = this_frame_values_binned[jj][ii];
				idx++;
			}
		}
			
		// 2.	Sort values and get [bg_level_quantile]th pixel value
		double this_frame_values_binned_1D_sorted [disp_nelements_binned*spat_nelements];
		memcpy(this_frame_values_binned_1D_sorted, this_frame_values_binned_1D, sizeof(double)*disp_nelements_binned*spat_nelements);		
		gsl_sort(this_frame_values_binned_1D_sorted, 1, disp_nelements_binned*spat_nelements);
		
		// 3.	Get background value
		double this_bg_values_mean = gsl_stats_mean(this_frame_values_binned_1D_sorted, 1, round(disp_nelements_binned*spat_nelements*bg_level_quantile));
		double this_bg_values_sd = gsl_stats_sd(this_frame_values_binned_1D_sorted, 1, round(disp_nelements_binned*spat_nelements*bg_level_quantile));
		
		printf("\nBinned background level determination");
		printf("\n-------------------------------------\n");
		printf("\nMean (counts):\t\t%.2f", this_bg_values_mean);
		printf("\nSD  (counts):\t\t%.2f", this_bg_values_sd);
		printf("\nTrig thresh (counts):\t> %.2f\n", this_bg_values_mean + scan_window_trig_nsigma*this_bg_values_sd);
		
		// FIND EDGES OF SPECTRUM
		// ***********************************************************************
		// 1.	Cycle through pixel value array
		
		double this_spat_values[spat_nelements];
		int this_el_disp, this_el_spat;
		
		double edges_t[disp_nelements_binned];
		int nedges_t = 0;
		double edges_b[disp_nelements_binned];
		int nedges_b = 0;	
		int kk;
		for (jj=0; jj<disp_nelements_binned; jj++) {
		
			memset(this_spat_values, 0, sizeof(double)*spat_nelements);
		
			// 2.	Accumulate values for this bin
			for (ii=0; ii<spat_nelements; ii++) {
				this_el_disp = jj;
				this_el_spat = ii;
				this_spat_values[ii] = this_frame_values_binned[this_el_spat][this_el_disp];
			}
			
			// 3.	FORWARD DIRECTION: Cycle through [this_spat_values] and for each pixel, check that all pixels within 
			// 	[scan_window_size_px] are succesively higher than [this_spat_values_sd] * 
			// 	[scan_window_trig_nsigma]. If true, flag as edge.
			int is_true_trigger;
			for (ii=0; ii<spat_nelements; ii++) {
				if (fabs(this_spat_values[ii] - this_bg_values_mean) > this_bg_values_sd*scan_window_trig_nsigma) {	// fire trigger
					is_true_trigger = true;					// assume it is a trigger pixel
					if (ii + scan_window_size_px >= spat_nelements) {	// check to make sure window doesn't fall off edge of frame
						is_true_trigger = false;
						break;
					} else {
						for (kk=ii; kk<ii+scan_window_size_px; kk++) {	// check pixels in window are greater than background
							if (fabs(this_spat_values[kk] - this_bg_values_mean) <= this_bg_values_sd*scan_window_trig_nsigma) {
								is_true_trigger = false;
								break;
							}
						}	
					}
					// store if true trigger
					if (is_true_trigger) {
						edges_b[nedges_b] = (double)ii;
						nedges_b++;
						break;
					}
				}						
			}
				
			// 4.	REVERSE DIRECTION: Cycle through [this_spat_values] and for each pixel, check that all pixels within 
			// 	[scan_window_size_px] are succesively higher than [this_spat_values_sd] * 
			// 	[scan_window_trig_nsigma]. If true, flag as edge.
			for (ii=spat_nelements-1; ii!=0; ii--) {
				if (fabs(this_spat_values[ii] - this_bg_values_mean) > this_bg_values_sd*scan_window_trig_nsigma) {	// fire trigger
					is_true_trigger = true;				// assume it is a trigger pixel
					if (ii - scan_window_size_px < 0) {		// check to make sure window doesn't fall off edge of frame
						is_true_trigger = false;
						break;
					} else {
						for (kk=ii; kk>ii-scan_window_size_px; kk--) {	// check pixels in window are greater than background
							if (fabs(this_spat_values[kk] - this_bg_values_mean) <= this_bg_values_sd*scan_window_trig_nsigma) {
								is_true_trigger = false;
								break;
							}
						}	
					}
					// store if true trigger
					if (is_true_trigger) {
						edges_t[nedges_t] = (double)ii;
						nedges_t++;						
						break;
					}
				}						
			}
		
		}
		
		int mean_edges_b = floor(gsl_stats_mean(edges_b, 1, nedges_b));
		int mean_edges_t = ceil(gsl_stats_mean(edges_t, 1, nedges_t));
		int mean_width = (int)fabs(mean_edges_t - mean_edges_b);
		
		printf("\nEdges detected");
		printf("\n--------------\n");
		printf("\nMean bottom position (px):\t%d", mean_edges_b);
		printf("\nMean top position (px):\t\t%d", mean_edges_t);	
		printf("\nAv width (px):\t\t\t%d\n", mean_width);
		
		if (mean_width < min_spectrum_width) {

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -14, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

			free(cont_f);
			free(in_f);					
			free(out_f);			
			if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 

			return 1; 		

		}
		
		// ***********************************************************************
		// Create [out_frame-values] array to hold the output data in the correct 
		// format

		double out_frame_values [disp_nelements * mean_width];
		memset(out_frame_values, 0, sizeof(double)*disp_nelements * mean_width);
		for (jj=mean_edges_b; jj<mean_width; jj++) {
	
			ii = jj * disp_nelements;
	
			for (kk=0; kk<disp_nelements; kk++) {
	
				out_frame_values[ii] = cont_frame_values[jj][kk];
				ii++;
	
			}
	
		}	
		
		// ***********************************************************************
		// Set output frame (ARG 8) parameters

		fitsfile *out_f_ptr;

		int out_f_status = 0;
		long out_f_naxes [2] = {disp_nelements, mean_width};
		long out_f_fpixel = 1;		
		
		// ***********************************************************************
		// Create and write trimmed file to output file (ARG 8)
	
		if (!fits_create_file(&out_f_ptr, out_f, &out_f_status)) {
	
			if (!fits_create_img(out_f_ptr, INTERMEDIATE_IMG_ACCURACY[0], 2, out_f_naxes, &out_f_status)) {

				if (!fits_write_img(out_f_ptr, INTERMEDIATE_IMG_ACCURACY[1], out_f_fpixel, disp_nelements * mean_width, out_frame_values, &out_f_status)) {

				} else { 

					write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -15, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

					free(cont_f);
					free(in_f);					
					free(out_f);			
					if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
					if(fits_close_file(out_f_ptr, &out_f_status)) fits_report_error (stdout, out_f_status); 					

					return 1; 	

				}

			} else {

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -16, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

				free(cont_f);
				free(in_f);					
				free(out_f);			
				if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 
				if(fits_close_file(out_f_ptr, &out_f_status)) fits_report_error (stdout, out_f_status); 
				
				return 1; 	

			}

		} else {

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -17, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

			free(cont_f);
			free(in_f);					
			free(out_f);			
			if(fits_close_file(cont_f_ptr, &cont_f_status)) fits_report_error (stdout, cont_f_status); 

			return 1; 	

		}	

		// ***********************************************************************
		// Clean up heap memory

		free(cont_f);
		free(in_f);					
		free(out_f);

		// ***********************************************************************
		// Close continuum file (ARG 1), input file (ARG 2) and output file 
		// (ARG 8)

		if(fits_close_file(cont_f_ptr, &cont_f_status)) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -18, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
			fits_report_error (stdout, cont_f_status); 

			return 1; 

	    	}
	    	
		if(fits_close_file(in_f_ptr, &in_f_status)) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -19, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
			fits_report_error (stdout, in_f_status); 

			return 1; 

	    	}
	    	
		if(fits_close_file(out_f_ptr, &out_f_status)) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", -20, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
			fits_report_error (stdout, out_f_status); 

			return 1; 

	    	}	    	

		// ***********************************************************************
		// Write success to [ERROR_CODES_FILE]

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATTR", RETURN_FLAG, "Status flag for L2 sptrim routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

		return 0;

	}

}


