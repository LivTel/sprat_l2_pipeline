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
#include "sprat_red_find.h"

#include <gsl/gsl_sort_double.h>
#include <gsl/gsl_statistics_double.h>

// *********************************************************************

int main(int argc, char *argv []) {

	if(populate_env_variable(REF_ERROR_CODES_FILE, "L2_ERROR_CODES_FILE")) {

		printf("\nUnable to populate [REF_ERROR_CODES_FILE] variable with corresponding environment variable. Routine will proceed without error handling\n");

	}

	if (argc != 5) {

		if(populate_env_variable(SPF_BLURB_FILE, "L2_SPF_BLURB_FILE")) {

			RETURN_FLAG = 1;

		} else {

			print_file(SPF_BLURB_FILE);

		}

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -1, "Status flag for spfind L2 routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

		return 1;

	} else {
		// ***********************************************************************
		// Redefine routine input parameters
		
		char *target_f				= strdup(argv[1]);	
		int bin_size_px				= strtol(argv[2], NULL, 0);
		int filter_width_px			= strtol(argv[3], NULL, 0);	
		int centroid_half_window_size_px	= strtol(argv[4], NULL, 0);
		
		// ***********************************************************************
		// Open target file (ARG 1), get parameters and perform any data format 
		// checks

		fitsfile *target_f_ptr;

		int target_f_maxdim = 2, target_f_status = 0, target_f_bitpix, target_f_naxis;
		long target_f_naxes [2] = {1,1};

		if(!fits_open_file(&target_f_ptr, target_f, IMG_READ_ACCURACY, &target_f_status)) {

			if(!populate_img_parameters(target_f, target_f_ptr, target_f_maxdim, &target_f_bitpix, &target_f_naxis, target_f_naxes, &target_f_status, "TARGET FRAME")) {

				if (target_f_naxis != 2) {	// any data format checks here

					write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -2, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

					free(target_f);
					if(fits_close_file(target_f_ptr, &target_f_status)) fits_report_error (stdout, target_f_status); 

					return 1;
	
				}

			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -3, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
				fits_report_error(stdout, target_f_status); 

				free(target_f);
				if(fits_close_file(target_f_ptr, &target_f_status)) fits_report_error (stdout, target_f_status); 

				return 1; 

			}

		} else { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -4, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
			fits_report_error(stdout, target_f_status); 

			free(target_f);

			return 1; 

		}
		
		// ***********************************************************************
		// Set the range limits

		int cut_x [2] = {1, target_f_naxes[0]};
		int cut_y [2] = {1, target_f_naxes[1]};

		// ***********************************************************************
		// Set parameters used when reading data from target file (ARG 1)

		long fpixel [2] = {cut_x[0], cut_y[0]};
		long nxelements = (cut_x[1] - cut_x[0]) + 1;
		long nyelements = (cut_y[1] - cut_y[0]) + 1;

		// ***********************************************************************
		// Create arrays to store pixel values from target fits file (ARG 1)

		double target_f_pixels [nxelements];
		
		// ***********************************************************************
		// Get target fits file (ARG 1) values and store in 2D array

		int ii;

		double target_frame_values [nyelements][nxelements];
		memset(target_frame_values, 0, sizeof(double)*nxelements*nyelements);
		for (fpixel[1] = cut_y[0]; fpixel[1] <= cut_y[1]; fpixel[1]++) {

			memset(target_f_pixels, 0, sizeof(double)*nxelements);

			if(!fits_read_pix(target_f_ptr, TDOUBLE, fpixel, nxelements, NULL, target_f_pixels, NULL, &target_f_status)) {

				for (ii=0; ii<nxelements; ii++) {

					target_frame_values[fpixel[1]-1][ii] = target_f_pixels[ii];

				}

			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -5, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
				fits_report_error(stdout, target_f_status); 

				free(target_f);									
				if(fits_close_file(target_f_ptr, &target_f_status)) fits_report_error (stdout, target_f_status); 

				return 1; 

			}

		}
		
		// FIND VALUES OF PEAK CENTROID ALONG DISPERSION AXIS
		// ***********************************************************************		
		// 1.	Bin array according to bin width given by [bin_size_px]
			
		int disp_nelements = nxelements, spat_nelements = nyelements;
		
		int disp_nelements_binned = (int)floor(disp_nelements/bin_size_px);			
		double this_frame_values_binned[spat_nelements][disp_nelements_binned];
		memset(this_frame_values_binned, 0, sizeof(double)*spat_nelements*disp_nelements_binned);
		
		double this_bin_value;
		int bin_number = 0;
		int jj;
		for (jj=0; jj<spat_nelements; jj++) {
			this_bin_value = 0;
			bin_number = 0;
			for (ii=0; ii<disp_nelements; ii++) {
				if (ii % bin_size_px == 0 && ii != 0) {
					this_frame_values_binned[jj][bin_number] = this_bin_value;
					bin_number++;
					this_bin_value = 0;
				}
				this_bin_value += target_frame_values[jj][ii];
			}
		}
	
		double this_spat_values[spat_nelements];
		double this_spat_values_smoothed[spat_nelements];
		double this_spat_values_smoothed_der[spat_nelements-1];
		
		double peaks[disp_nelements_binned];
		for (ii=0; ii<disp_nelements_binned; ii++) {
			// 2.	Smooth array with median filter
			for (jj=0; jj<spat_nelements-1; jj++) {
				this_spat_values[jj] = this_frame_values_binned[jj][ii];
			}
			
			memset(this_spat_values_smoothed, 0, sizeof(double)*spat_nelements-1);
			median_filter(this_spat_values, this_spat_values_smoothed, spat_nelements-1, filter_width_px);
			
			// 3.	Establish if any target flux is in this bin
			int retain_indexes[spat_nelements-1];
			double final_mean, final_sd;
			int final_num_retained_indexes;
			
			iterative_sigma_clip(this_spat_values_smoothed, spat_nelements-1, 0.5, retain_indexes, &final_mean, &final_sd, &final_num_retained_indexes, FALSE);
			if (final_num_retained_indexes == spat_nelements-1) {
				peaks[ii] = -1;		// no clipping occurred, couldn't find a target
				continue;
			}
			
			// 3.	Take derivatives
			memset(this_spat_values_smoothed_der, 0, sizeof(double)*spat_nelements);
			for (jj=0; jj<spat_nelements-1; jj++) {
				this_spat_values_smoothed_der[jj] = this_frame_values_binned[jj][ii] - this_frame_values_binned[jj-1][ii];
			}
			
			// 4.	Get index of sorted derivatives (in ascending order) and pick most negative gradient
			size_t this_spat_values_smoothed_der_idx [spat_nelements-1];	
			gsl_sort_index(this_spat_values_smoothed_der_idx, this_spat_values_smoothed_der, 1, spat_nelements-1);
			double this_pk_idx = this_spat_values_smoothed_der_idx[spat_nelements-3] + 2;	// +2 as correction for taking derivative
			peaks[ii] = this_pk_idx;
		}	
		
		// 5.	Get the median value of peak array
		double peaks_sorted[disp_nelements_binned];
		memcpy(peaks_sorted, peaks, sizeof(double)*disp_nelements_binned);		
		gsl_sort(peaks_sorted, 1, disp_nelements_binned);	

		int pk_idx_median = (int)round(gsl_stats_median_from_sorted_data(peaks_sorted, 1, disp_nelements_binned));
		
		printf("\nPeak finding results");
		printf("\n--------------------\n");
		printf("\nMedian peak index (px):\t%d\n", pk_idx_median);

		// 6.	Get parabolic centroid
		double this_pk_window_idxs[1 + (2*centroid_half_window_size_px)];
		double this_pk_window_vals[1 + (2*centroid_half_window_size_px)];
		double peaks_fitted[disp_nelements_binned];
		for (ii=0; ii<disp_nelements_binned; ii++) {
			if (peaks[ii] == -1)
				continue;
			
			memset(this_pk_window_idxs, 0, sizeof(double)*1 + (2*centroid_half_window_size_px));
			memset(this_pk_window_vals, 0, sizeof(double)*1 + (2*centroid_half_window_size_px));
			int idx = 0;
			for (jj=pk_idx_median-centroid_half_window_size_px; jj<=pk_idx_median+centroid_half_window_size_px; jj++) {
				this_pk_window_idxs[idx] = jj;
				this_pk_window_vals[idx] = this_frame_values_binned[jj][ii];
				idx++;
			}

			int order = 2;
			double coeffs [order+1];  
			memset(coeffs, 0, sizeof(double)*order+1);
			double chi_squared;
			if (calc_least_sq_fit(2, 1 + (2*centroid_half_window_size_px), this_pk_window_idxs, this_pk_window_vals, coeffs, &chi_squared)) {

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -6, "Status flag for L2 frfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

				free(target_f);
				if(fits_close_file(target_f_ptr, &target_f_status)) fits_report_error (stdout, target_f_status); 

				return 1; 		

			}

			peaks_fitted[ii] = -coeffs[1]/(2*coeffs[2]);

		}

		// ***********************************************************************
		// Create [FRFIND_OUTPUTF_PEAKS_FILE] output file and print a few 
		// parameters

		FILE *outputfile;
		outputfile = fopen(SPFIND_OUTPUTF_PEAKS_FILE, FILE_WRITE_ACCESS);

		if (!outputfile) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -7, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

			free(target_f);
			if(fits_close_file(target_f_ptr, &target_f_status)) fits_report_error (stdout, target_f_status); 

			return 1;


		}

		char timestr [80];
		memset(timestr, '\0', sizeof(char)*80);

		find_time(timestr);

		fprintf(outputfile, "#### %s ####\n\n", SPFIND_OUTPUTF_PEAKS_FILE);
		fprintf(outputfile, "# Lists the coordinates of the peaks found using the spfind routine.\n\n");
		fprintf(outputfile, "# Run filename:\t%s\n", target_f);
		fprintf(outputfile, "# Run datetime:\t%s\n\n", timestr);
		
		for (ii=0; ii<disp_nelements_binned; ii++) {
			if (peaks[ii] == -1)
				continue;
			
			fprintf(outputfile, "%f\t%f\n", ii*bin_size_px + (double)bin_size_px/2., peaks_fitted[ii]);
		}
		
		fprintf(outputfile, "%d", EOF);		
		
		// ***********************************************************************
		// Clean up heap memory

		free(target_f);

		// ***********************************************************************
		// Close [FRFIND_OUTPUTF_PEAKS_FILE] output file and target file (ARG 1)
		
		if (fclose(outputfile)) {

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -8, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

			if(fits_close_file(target_f_ptr, &target_f_status)) fits_report_error (stdout, target_f_status); 

			return 1; 

		}

		if(fits_close_file(target_f_ptr, &target_f_status)) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", -9, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);
			fits_report_error (stdout, target_f_status); 

			return 1; 

	    	}		
		
		// ***********************************************************************
		// Write success to [ERROR_CODES_FILE]

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFI", RETURN_FLAG, "Status flag for L2 spfind routine", ERROR_CODES_INITIAL_FILE_WRITE_ACCESS);

		return 0;

	}

}


