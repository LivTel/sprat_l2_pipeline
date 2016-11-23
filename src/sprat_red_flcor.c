/************************************************************************

 File:				sprat_red_flcor.c
 Last Modified Date:     	22/11/2016

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
#include "sprat_red_flcor.h"
#include "sprat_red_arcfit.h"

#include <gsl/gsl_math.h>

// *********************************************************************/

int main (int argc, char *argv []) {

	int ii,jj,kk; 				// counters for short term use in loops

	if(populate_env_variable(REF_ERROR_CODES_FILE, "L2_ERROR_CODES_FILE")) {

		printf("\nUnable to populate [REF_ERROR_CODES_FILE] variable with corresponding environment variable. Routine will proceed without error handling\n");

	}

	if (argc != 11) {

		if(populate_env_variable(SPFLCOR_BLURB_FILE, "L2_SPFLCOR_BLURB_FILE")) {

			RETURN_FLAG = 1;

		} else {

			print_file(SPFLCOR_BLURB_FILE);

		}

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -1, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);

		return 1;

	} else {

		// ***********************************************************************
		// Redefine routine input parameters
	
		char *input_f	                = strdup(argv[1]);		// "input" relates to the spectrum
		float input_exptime		= strtod(argv[2],NULL);		// "input" relates to the spectrum
		float input_start_wav 		= strtod(argv[3],NULL);		// "input" relates to the spectrum
		float input_dispersion 		= strtod(argv[4],NULL);		// "input" relates to the spectrum
		float acq_image_counts		= strtod(argv[5],NULL);		// "acq" relates to the final acquisition image
		float acq_wav_min		= strtod(argv[6],NULL);		// "acq" relates to the final acquisition image
		float acq_wav_max		= strtod(argv[7],NULL);		// "acq" relates to the final acquisition image
		char *flcor_f	                = strdup(argv[8]);
		float tel_thput			= strtod(argv[9],NULL);		// "tel" relates to the telescope
		char *output_f	                = strdup(argv[10]);

		// input_exptime exists for historical reasons when we originally applied an absolute calibration
		// instead of normalising to the final acquisiton image. It is no longer used.
		input_exptime = 1;


		// ***********************************************************************
		// Ensure the input files exist

                if (check_file_exists(input_f)) {
                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -4, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                        return 1;
                }
                if (check_file_exists(flcor_f)) {
                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -7, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                        return 1;
                }

		// ***********************************************************************
		// Open input file (ARG 1), get parameters and perform any data format 
                // checks 

		fitsfile *input_f_ptr;

		int input_f_maxdim = 2;
		int input_f_status = 0, input_f_bitpix, input_f_naxis;
		long input_f_naxes [2] = {1,1};



		if(!fits_open_file(&input_f_ptr, input_f, READONLY, &input_f_status)) {

			if(!populate_img_parameters(input_f, input_f_ptr, input_f_maxdim, &input_f_bitpix, &input_f_naxis, input_f_naxes, &input_f_status, "INPUT FRAME")) {

				if (input_f_naxis != 2) {	// any data format checks here

					write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -2, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);

					free(input_f);
					free(flcor_f);
					free(output_f);

					if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 

					return 1;
	
				}

			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -3, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
				fits_report_error(stdout, input_f_status); 

				free(input_f);
				free(flcor_f);
				free(output_f);

				if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 

				return 1; 

			}

		} else { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -4, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fits_report_error(stdout, input_f_status); 

			free(input_f);
			free(flcor_f);
			free(output_f);

			return 1; 

		}


		// ***********************************************************************
		// Open flcor file (ARG 2), get parameters and perform any data format checks 

		fitsfile *flcor_f_ptr;

		int flcor_f_maxdim = 2;
		int flcor_f_status = 0, flcor_f_bitpix, flcor_f_naxis;
		long flcor_f_naxes [2] = {1,1};

		if(!fits_open_file(&flcor_f_ptr, flcor_f, READONLY, &flcor_f_status)) {

			if(!populate_img_parameters(flcor_f, flcor_f_ptr, flcor_f_maxdim, &flcor_f_bitpix, &flcor_f_naxis, flcor_f_naxes, &flcor_f_status, "FLCOR FRAME")) {

				if (flcor_f_naxis != 2 || (flcor_f_naxes[0]!=input_f_naxes[0]) || (flcor_f_naxes[1]!=1) ) {	// any data format checks here

					write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -5, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);

                                	free(input_f);
					free(flcor_f);
                                	free(output_f);

					if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
					if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 

					return 1;
	
				}


			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -6, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
				fits_report_error(stdout, flcor_f_status); 

				free(input_f);
				free(flcor_f);
				free(output_f);

				if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
				if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 

				return 1; 

			}

		} else { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -7, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fits_report_error(stdout, flcor_f_status); 

			free(input_f);
			free(flcor_f);
			free(output_f);

			return 1; 

		}


		// ***********************************************************************
		// Set the range limits using input fits file (ARG 1)
		// These can be based on input because we have already confirmed that flcor agrees 

		int cut_x [2] = {1, input_f_naxes[0]};
		int cut_y [2] = {1, input_f_naxes[1]};

		// ***********************************************************************
		// Set parameters used when reading data from input fits file (ARG 1)

		long fpixel [2] = {cut_x[0], cut_y[0]};
		long nxelements = (cut_x[1] - cut_x[0]) + 1;
		long nyelements = (cut_y[1] - cut_y[0]) + 1;

		// ***********************************************************************
		// Create arrays to store pixel values from both input fits files

		double input_f_pixels [nxelements];
		double flcor_f_pixels [nxelements];
		double *ptr_flcor, *ptr_input, *ptr_output;


		int this_row_index;

		double output_frame_values [nyelements][nxelements];
		memset(output_frame_values, 0, sizeof(double)*nyelements*nxelements);

		double spec_renormalize = 0;

		// Pixel range of the spectrum to use in the normalisation
		int norm_range_low, norm_range_high;


		// ***********************************************************************
		// 1.	Read the pixel values from the flcor frame
		//
		// The flux calibration as read from flcor converts from 
		//	ADU / sec on sprat
		// to
		//	flux density in W/m2/A
		// I.e., Simply multiplying a 1sec SPRAT SPEC_SS by the transformation curve will
		// yield the full calibration.
		//
		// There is no need to apply any corrections in the code here for pixel width (dispersion)
		// because that is already implicit in the flcor file. Such compensations would need to be
		// added to this code if SPRAT configs were highly variable. As it is we only have the two
		// basic configs and they are stable so it is easier to roll the dispersion terms into the
		// calibration frame and require the flcor file to be identical wavelength range, dispersion
		// etc as the data.
		//
		// However, the calibration is only correct at the time it is made. As the telescope primary
		// mirror degrades over time, the absolute calibration will drift. Assuming the degradation
		// is perfectly grey, all we need is a correction factor that says what the
		// total throughput of the system now as a fraction of the throughput when the calibration was
		// made. Whenever a new flcor calibration is created this gets reset to unity since flcor
		// is then correct. The scaling factor can slowly be incremented over time in the config.ini
		// to account for the degrading mirror. If the mirror degradation is not grey, then flcor
		// simply needs more frequent updates.

		this_row_index = 0;

		memset(flcor_f_pixels, 0, sizeof(double)*nxelements);

		if(fits_read_pix(flcor_f_ptr, IMG_READ_ACCURACY, fpixel, nxelements, NULL, flcor_f_pixels, NULL, &flcor_f_status)) {

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -9, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fits_report_error(stdout, input_f_status);

			free(input_f);
			free(flcor_f);
			free(output_f);

			if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
			if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 

			return 1; 
		}

		// Apply the telescope throughput correction. 
		// This accounts for the change in throughput of the telescope since the flcor calibration was created.
		// When using a recent flcor it will simply be 1.
		if (tel_thput != 1 && tel_thput != 0) {
			ptr_flcor = flcor_f_pixels;
			for (ii = 0; ii < nxelements; ii++) {
			  *ptr_flcor /= tel_thput;
			  ptr_flcor++;
			}
		}


		// ***********************************************************************
		// 2.	Read the input frame

		for (fpixel[1] = cut_y[0]; fpixel[1] <= cut_y[1]; fpixel[1]++) {

			this_row_index = fpixel[1] - 1;

			memset(input_f_pixels, 0, sizeof(double)*nxelements);

			if(!fits_read_pix(input_f_ptr, IMG_READ_ACCURACY, fpixel, nxelements, NULL, input_f_pixels, NULL, &input_f_status)) {


				// ***********************************************************************
				// 3. Populate output_frame_values with ( input_f_pixels * flcor_f_pixels )
				//    That gets the spectral shape correct
				ptr_flcor = flcor_f_pixels;
				ptr_input = input_f_pixels;
				ptr_output = &(output_frame_values[this_row_index][0]);
				for (ii = 0; ii < nxelements; ii++) {
					*ptr_output = (*ptr_input) * (*ptr_flcor);
					ptr_flcor++;
					ptr_input++;
					ptr_output++;
				}

				// ***********************************************************************
				// 4. Normalise the corrected spectrum in one of two different ways
				//
				//    acq_image_counts == 0
				//	Normalize in the range FLAMBDA_NORM_RANGE_LOW < lambda < FLAMBDA_NORM_RANGE_HIGH
				//	which gives us F_lambda, but no absolute flux calibration units
				//
				//    acq_image_counts != 0
				//	Use counts from the acquisition image to get a true flux density calibration

				if (acq_image_counts == 0) {

				  // FLAMBDA_NORM_RANGE_LOW, FLAMBDA_NORM_RANGE_HIGH are set in the header file. 
				  // Currently hard coded in the header file. Would be easy to move into config.ini
				  norm_range_low = (int)fmax(0 , floor((FLAMBDA_NORM_RANGE_LOW - input_start_wav) / input_dispersion));
				  norm_range_high = (int)fmin(nxelements , ceil((FLAMBDA_NORM_RANGE_HIGH - input_start_wav) / input_dispersion));
				  spec_renormalize = 0;
				  ptr_output = &(output_frame_values[this_row_index][norm_range_low]);
				  for (ii = norm_range_low; ii < norm_range_high; ii++) {
					spec_renormalize += (*ptr_output);
 					ptr_output++;
				  }
				  spec_renormalize /= (norm_range_high - norm_range_low);
				  if (spec_renormalize != 0)
				    spec_renormalize = 1.0 / spec_renormalize;
				  else
				    spec_renormalize = 1;

				} else {

				  // Sum up total contents of the spectrum as read so it can be scaled to match acq_image_counts from acq image
				  // We only sum up that part of the spectrum lying between wavelengths acq_wav_min -> acq_wav_max.
				  // This allows the scaling to apply only to the filter transmission band of the acquisition camera
				  // If acq_wav_min == 0 sum from the blue end of the spectrum up
				  // If acq_wav_max == 0 sum to the red end of the spectrum

				  norm_range_low = (int)fmax(0 , floor((acq_wav_min - input_start_wav) / input_dispersion));

				  if ( acq_wav_max == 0 )
				    norm_range_high = nxelements;
				  else
				    norm_range_high = (int)fmin(nxelements , ceil((acq_wav_max - input_start_wav) / input_dispersion));

				  ptr_input = &(input_f_pixels[norm_range_low]);
				  spec_renormalize = 0;
				  for (ii = norm_range_low; ii < norm_range_high; ii++) {
					spec_renormalize += (*ptr_input);
 					ptr_input++;
				  }
				  // Scale to match spectrum and acquisition image
				  //spec_renormalize = input_dispersion * acq_image_counts / spec_renormalize ;
				  spec_renormalize = acq_image_counts / spec_renormalize ;

				  // Note that EXPTIME is not used here. That is counter intuitive for photometry. If
				  // we did blind absolute phtometry then we would need to scale by EXPTIME here. In fact
				  // we normalise to the acquisition image. After the normalisation, what we have in our
				  // extracted file is a spectrum with the same total ADU as a 1sec image and we have removed 
				  // all slit and grating losses. The calibration curve is defined to take that 
				  // as it's input. The calibration converts from ADU/sec in the acq image to flux density in
				  // the spectrum. 
				  // EXPTIME is passed into this code for historical reasons and may 
				  // be useful in the future if we return to absolute photometry. 

				}

				// Apply the normalisation
                                ptr_output = &(output_frame_values[this_row_index][0]);
                                for (ii = 0; ii < nxelements; ii++) {
                                        *ptr_output = (*ptr_output) * spec_renormalize;
                                        ptr_output++;
                                }


			} else { 

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -10, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
				fits_report_error(stdout, input_f_status);

				free(input_f);
				free(flcor_f);
				free(output_f);

				if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
				if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 

				return 1; 

			}

		}



		// 5.	Create [SPFLCOR_OUTPUTF] log output file and print
		// 	a few parameters

		FILE *outputfile;
		outputfile = fopen(SPFLCOR_OUTPUTF, FILE_WRITE_ACCESS);

		if (!outputfile) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -11, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);

			free(input_f);
			free(flcor_f);
			free(output_f);

			if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
			if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 

			return 1;

		}

		char timestr [80];
		memset(timestr, '\0', sizeof(char)*80);

		find_time(timestr);

		fprintf(outputfile, "#### %s ####\n\n", SPFLCOR_OUTPUTF);
	        fprintf(outputfile, "# Throughput correction parameters.\n\n");
                fprintf(outputfile, "# Run Datetime:\t\t%s\n\n", timestr);
	        fprintf(outputfile, "# Target Filename:\t%s\n\n", input_f);
	        fprintf(outputfile, "# Throughput Filename:\t%s\n\n", flcor_f);
		fprintf(outputfile, "%d", EOF);

		// 6.	Write these values to the [ADDITIONAL_KEYS_FILE] file

		write_additional_key_to_file_str(ADDITIONAL_KEYS_FILE, "FLCOR_CALIBRATION", "L2FLCOR", flcor_f, "Filename of throughput calibration", ADDITIONAL_KEYS_FILE_WRITE_ACCESS);




		// ***********************************************************************
		// Set output frame parameters

		fitsfile *output_f_ptr;
	
		int output_f_status = 0;
		long output_f_naxes [2] = {nxelements,nyelements};
		long output_f_fpixel = 1;

		// ***********************************************************************
		// Create [output_frame_values_1D] array to hold the output data in the 
                // correct format

		double output_frame_values_1D [nxelements*nyelements];
		memset(output_frame_values_1D, 0, sizeof(double)*nxelements*nyelements);
		for (ii=0; ii<nyelements; ii++) {
	
			jj = ii * nxelements;
	
			for (kk=0; kk<nxelements; kk++) {
	
				output_frame_values_1D[jj] = output_frame_values[ii][kk];
				jj++;

			}
		
		}

		// ***********************************************************************
		// Create and write [output_frame_values_1D] to output file (ARG 4)	
	
		if (!fits_create_file(&output_f_ptr, output_f, &output_f_status)) {
	
			if (!fits_create_img(output_f_ptr, INTERMEDIATE_IMG_ACCURACY[0], 2, output_f_naxes, &output_f_status)) {

				if (!fits_write_img(output_f_ptr, INTERMEDIATE_IMG_ACCURACY[1], output_f_fpixel, nxelements * nyelements, output_frame_values_1D, &output_f_status)) {

				} else { 

					write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -12, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
					fits_report_error(stdout, output_f_status); 

					free(input_f);
					free(flcor_f);
					free(output_f);

					fclose(outputfile);

					if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
					if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 
					if(fits_close_file(output_f_ptr, &output_f_status)); 

					return 1; 

				}

			} else {

				write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -13, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
				fits_report_error(stdout, output_f_status); 

				free(input_f);
				free(flcor_f);
				free(output_f);

				fclose(outputfile);

                                if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
                                if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 
                                if(fits_close_file(output_f_ptr, &output_f_status)); 

				return 1; 

			}

		} else {

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -14, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fits_report_error(stdout, output_f_status); 

			free(input_f);
			free(flcor_f);
			free(output_f);

			fclose(outputfile);

			if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status);  
			if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status);  

			return 1; 

		}

		// ***********************************************************************
		// Clean up heap memory

		free(input_f);
		free(flcor_f);
		free(output_f);

		// ***********************************************************************
		// Close input file (ARG 1), flcor file (ARC 2), output file (ARG 4) and [SPFLCOR_OUTPUTF] log file

		// The log file
		if (fclose(outputfile)) {

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -15, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);

                        if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
                        if(fits_close_file(flcor_f_ptr, &flcor_f_status)) fits_report_error (stdout, flcor_f_status); 
                        if(fits_close_file(output_f_ptr, &output_f_status)); 

			return 1; 

		}

		// The flcor file
		if(fits_close_file(flcor_f_ptr, &flcor_f_status)) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -16, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fits_report_error (stdout, flcor_f_status); 

                        if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
                        if(fits_close_file(output_f_ptr, &output_f_status)); 

			return 1; 

	    	}

		// The input file
		if(fits_close_file(input_f_ptr, &input_f_status)) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -17, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fits_report_error (stdout, input_f_status); 

                        if(fits_close_file(output_f_ptr, &output_f_status)); 

			return 1; 

	    	}

		// The resulting output FITS
		if(fits_close_file(output_f_ptr, &output_f_status)) { 

			write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", -18, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);
			fits_report_error (stdout, output_f_status); 

			return 1; 

	    	}

		// ***********************************************************************
		// Write success to [ERROR_CODES_FILE]

		write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATFL", RETURN_FLAG, "Status flag for L2 spflcor routine", ERROR_CODES_FILE_WRITE_ACCESS);

		return 0;

	}

}

