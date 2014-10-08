/************************************************************************

 File:                          sprat_red_extract.c
 Last Modified Date:            07/10/14

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
#include "sprat_red_extract.h"
#include "sprat_red_trace_sdist.h"
#include "gsl_poly.h"

// *********************************************************************

int main(int argc, char *argv []) {

        if(populate_env_variable(REF_ERROR_CODES_FILE, "L2_ERROR_CODES_FILE")) {

                printf("\nUnable to populate [REF_ERROR_CODES_FILE] variable with corresponding environment variable. Routine will proceed without error handling\n");

        }

        if (argc != 7) {

                if(populate_env_variable(SPE_BLURB_FILE, "L2_SPE_BLURB_FILE")) {

                        RETURN_FLAG = 1;

                } else {

                        print_file(SPE_BLURB_FILE);

                }

                write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -1, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);

                return 1;

        } else {
                // ***********************************************************************
                // Redefine routine input parameters
                
                char *input_f                           = strdup(argv[1]);         
                double spectrum_px                      = strtod(argv[2], NULL);      
                char *method                            = strdup(argv[3]); 
                double target_half_aperture_px          = strtod(argv[4], NULL); 
                double sky_window_px                    = strtod(argv[5], NULL);   
                char *output_f                          = strdup(argv[6]);                    
                
                // ***********************************************************************
                // Open input file (ARG 1), get parameters and perform any data format 
                // checks

                fitsfile *input_f_ptr;

                int input_f_maxdim = 2, input_f_status = 0, input_f_bitpix, input_f_naxis;
                long input_f_naxes [2] = {1,1};

                if(!fits_open_file(&input_f_ptr, input_f, IMG_READ_ACCURACY, &input_f_status)) {

                        if(!populate_img_parameters(input_f, input_f_ptr, input_f_maxdim, &input_f_bitpix, &input_f_naxis, input_f_naxes, &input_f_status, "INPUT FRAME")) {

                                if (input_f_naxis != 2) {       // any data format checks here

                                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -2, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);

                                        free(input_f);
                                        free(output_f);
                                        free(method);
                                        if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 

                                        return 1;
        
                                }

                        } else { 

                                write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -3, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                                fits_report_error(stdout, input_f_status); 

                                free(input_f);
                                free(output_f);                                
                                free(method);
                                if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 

                                return 1; 

                        }

                } else { 

                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -4, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                        fits_report_error(stdout, input_f_status); 

                        free(input_f);
                        free(output_f);                        
                        free(method);
                        
                        return 1; 

                }
                
                // ***********************************************************************
                // Set the range limits

                int cut_x [2] = {1, input_f_naxes[0]};
                int cut_y [2] = {1, input_f_naxes[1]};

                // ***********************************************************************
                // Set parameters used when reading data from input file (ARG 1)

                long fpixel [2] = {cut_x[0], cut_y[0]};
                long nxelements = (cut_x[1] - cut_x[0]) + 1;
                long nyelements = (cut_y[1] - cut_y[0]) + 1;

                // ***********************************************************************
                // Create arrays to store pixel values from input fits file (ARG 1)

                double input_f_pixels [nxelements];
                
                // ***********************************************************************
                // Get input fits file (ARG 1) values and store in 2D array

                int ii;

                double input_frame_values [nyelements][nxelements];
                memset(input_frame_values, 0, sizeof(double)*nxelements*nyelements);
                for (fpixel[1] = cut_y[0]; fpixel[1] <= cut_y[1]; fpixel[1]++) {

                        memset(input_f_pixels, 0, sizeof(double)*nxelements);

                        if(!fits_read_pix(input_f_ptr, TDOUBLE, fpixel, nxelements, NULL, input_f_pixels, NULL, &input_f_status)) {

                                for (ii=0; ii<nxelements; ii++) {
                                        input_frame_values[fpixel[1]-1][ii] = input_f_pixels[ii];
                                }

                        } else { 

                                write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -5, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                                fits_report_error(stdout, input_f_status); 

                                free(input_f);  
                                free(output_f);  
                                free(method);                                
                                if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 

                                return 1; 

                        }

                }
                
                double output_frame_values[nxelements];
                memset(output_frame_values, 0, sizeof(double)*nxelements);                       
                if (strcmp(method, "simple") == 0) {
                    // ***********************************************************************
                    // PARTIAL PIXEL APERTURE EXTRACTION OF FLUX, NO SKY SUBTRACTION

                    int ii, jj;

                    double y;    
                    
                    y = spectrum_px;
     
                    double this_col_value;
                    for (ii=0; ii<nxelements; ii++) {   
                      
                        this_col_value = 0.;
                        
                        // ***********************************************************************
                        // Does [y] violate the img boundaries?

                        if ((y + target_half_aperture_px > nyelements) || (y - target_half_aperture_px <= 0)) {

                            write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -6, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                            fits_report_error(stdout, input_f_status); 

                            free(input_f);
                            free(output_f);  
                            free(method);
                            if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 

                            return 1;

                        }

                        // ***********************************************************************
                        // Extract flux within aperture

                        double y_low, y_high;

                        y_low = y-target_half_aperture_px-0.5;
                        y_high = y+target_half_aperture_px+0.5;                   

                        int y_low_floor, y_high_floor;
        
                        y_low_floor = floor(y-target_half_aperture_px-0.5);
                        y_high_floor = floor(y+target_half_aperture_px+0.5);
                                            
                        for (jj=y_low_floor; jj<=y_high_floor; jj++) {
                          
                            if (jj == y_low_floor) {                        // outside pixel where partial flux needs to be taken into account
                                  
                                double partial_fraction_of_bin = (y_low_floor + 1) - y_low;
                                this_col_value += partial_fraction_of_bin * input_frame_values[jj][ii];
                                    
                            } else if (jj == y_high_floor) {                // outside pixel where partial flux needs to be taken into account
                                  
                                double partial_fraction_of_bin = y_high - y_high_floor;
                                this_col_value += partial_fraction_of_bin * input_frame_values[jj][ii];

                            } else {
                              
                                this_col_value += input_frame_values[jj][ii]; 
                                    
                            }
                        }  
                        
                        output_frame_values[ii] = this_col_value;
                    }    
                } else {
                  
                    write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -7, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                    fits_report_error(stdout, input_f_status); 

                    free(input_f);
                    free(output_f);
                    free(method);
                    if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status);  
                    
                    return 1;
                    
                }
                
                // ***********************************************************************
                // Set output frame parameters

                fitsfile *output_f_ptr;
        
                int output_f_status = 0;
                long output_f_naxes [2] = {nxelements, 1};
        
                long output_f_fpixel = 1;

                // ***********************************************************************
                // Create and write [output_frame_values] to output file (ARG 6)
        
                if (!fits_create_file(&output_f_ptr, output_f, &output_f_status)) {
        
                        if (!fits_create_img(output_f_ptr, INTERMEDIATE_IMG_ACCURACY[0], 2, output_f_naxes, &output_f_status)) {

                                if (!fits_write_img(output_f_ptr, INTERMEDIATE_IMG_ACCURACY[1], output_f_fpixel, nxelements, output_frame_values, &output_f_status)) {

                                } else { 

                                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -8, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                                        fits_report_error(stdout, output_f_status); 

                                        free(input_f);
                                        free(output_f);
                                        free(method);
                                        if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
                                        if(fits_close_file(output_f_ptr, &output_f_status)) fits_report_error (stdout, output_f_status);

                                        return 1; 

                                }

                        } else {

                                write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -9, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                                fits_report_error(stdout, output_f_status); 

                                free(input_f);
                                free(output_f);
                                free(method);
                                if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 
                                if(fits_close_file(output_f_ptr, &output_f_status)) fits_report_error (stdout, output_f_status);

                                return 1; 

                        }

                } else {

                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -10, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                        fits_report_error(stdout, output_f_status); 

                        free(input_f);
                        free(output_f);
                        free(method);
                        if(fits_close_file(input_f_ptr, &input_f_status)) fits_report_error (stdout, input_f_status); 

                        return 1; 

                }                
                
                // ***********************************************************************
                // Free arrays on heap

                free(input_f);
                free(output_f);
                free(method);                
                
                // ***********************************************************************
                // Close input file (ARG 1) and output file (ARG 6)          
                
                if(fits_close_file(input_f_ptr, &input_f_status)) { 

                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -11, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                        fits_report_error (stdout, input_f_status); 

                        return 1; 

                }     
               
                if(fits_close_file(output_f_ptr, &output_f_status)) { 

                        write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", -12, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);
                        fits_report_error (stdout, output_f_status); 

                        return 1; 

                }  
                
                // Write success to [ERROR_CODES_FILE]

                write_key_to_file(ERROR_CODES_FILE, REF_ERROR_CODES_FILE, "L2STATEX", RETURN_FLAG, "Status flag for L2 spextract routine", ERROR_CODES_FILE_WRITE_ACCESS);

                return 0;

        }

}

