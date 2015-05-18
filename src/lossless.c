#include <setjmp.h>
#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>

#include "lossless.h"
#include "utils.h"

int cclt_optimize(char* input_file, char* output_file, int exif_flag) {
	//File pointer for both input and output
	FILE* fp;
	
	//Those will hold the input/output structs
	struct jpeg_decompress_struct srcinfo;
  	struct jpeg_compress_struct dstinfo;
  	
	//Error Handling
  	struct jpeg_error_mgr jsrcerr, jdsterr;

	//Input/Output array coefficents
  	jvirt_barray_ptr* src_coef_arrays;
  	jvirt_barray_ptr* dst_coef_arrays;

	//Set errors and create the compress/decompress istances
  	srcinfo.err = jpeg_std_error(&jsrcerr);
  	jpeg_create_decompress(&srcinfo);
  	dstinfo.err = jpeg_std_error(&jdsterr);
  	jpeg_create_compress(&dstinfo);
  	
  	//Open the input file
  	fp = fopen(input_file, "r");
  	
	//Check for errors
	//TODO Use UNIX error messages
	if (fp == NULL) {
    	printf("INPUT: Failed to open file \"%s\"\n", input_file);
    	return -1;
    }
	
	//Create the IO istance for the input file
    jpeg_stdio_src(&srcinfo, fp);

    //Save EXIF info
    if (exif_flag == 1) {
    	for (int m = 0; m < 16; m++) {
      	jpeg_save_markers(&srcinfo, JPEG_APP0 + m, 0xFFFF);
  		}
 	}
    
    //Read the input headers
  	(void) jpeg_read_header(&srcinfo, TRUE);

	//Read input coefficents
  	src_coef_arrays = jpeg_read_coefficients(&srcinfo);
  	//jcopy_markers_setup(&srcinfo, copyoption);
	
	//Copy parameters
  	jpeg_copy_critical_parameters(&srcinfo, &dstinfo);

	//Set coefficents array to be the same
    dst_coef_arrays = src_coef_arrays;

	//We don't need the input file anymore
    fclose(fp);

	//Open the output one instead
    fp = fopen(output_file, "w+");
	//Check for errors
	//TODO Use UNIX error messages
	if (fp == NULL) {
    	printf("OUTPUT: Failed to open file \"%s\"\n", output_file);
    	return -2;
    }
	
	//CRITICAL - This is the optimization step
    dstinfo.optimize_coding = TRUE;
    jpeg_simple_progression(&dstinfo);

	//Set the output file parameters
    jpeg_stdio_dest(&dstinfo, fp);
    
	
	//Actually write the coefficents
    jpeg_write_coefficients(&dstinfo, dst_coef_arrays);

    //Write EXIF
    if (exif_flag == 1) {
    	jcopy_markers_execute(&srcinfo, &dstinfo);
	}
    
    //Free memory and finish
    jpeg_finish_compress(&dstinfo);
	jpeg_destroy_compress(&dstinfo);
	(void) jpeg_finish_decompress(&srcinfo);
	jpeg_destroy_decompress(&srcinfo);

	//Close the output file
	fclose(fp);

	return 0;
}
