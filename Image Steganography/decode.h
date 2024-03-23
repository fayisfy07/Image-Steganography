#ifndef DECODE_H
#define DECODE_H

#include "types.h"

#define MAX_OUTPUT_BUF_SIZE 1
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
	/*Source Image Info*/
	char *src_image_fname;
	FILE *fptr_src_image;
	
	/* Output File Info */
	char output_fname[30];
	FILE *fptr_output;
	char extn_secret_file[MAX_FILE_SUFFIX];
	char secret_data[MAX_OUTPUT_BUF_SIZE];
	long size_output_file;
	long output_file_extn_size;
	
	/* Magic String Signature */
	char *magic_string_sig;
	long magic_str_length;
} DecodeInfo;

/* Decoding function prototypes */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args( char *argv[], DecodeInfo *decInfo);	//to decode validate list

/* Open the stego Files */
Status open_file_decode( DecodeInfo *decInfo );

/* Perform the decoding operation */
Status do_decoding( DecodeInfo *decInfo );

/* Skip BMP header data */
Status skip_bmp_header( FILE *fptr );

/* Decoding the magic string data */
Status decode_magic_string( DecodeInfo *decInfo , const char *magic_string );

/* Decode output file extension size */
Status decode_output_file_extn_size( DecodeInfo *decInfo);

/* Decode output file extension */
Status decode_output_file_extn( char *extn , DecodeInfo *decInfo);

/* Create and open output file */
Status create_and_open_output_file( DecodeInfo *decInfo );

/* Decode output file size */ 
Status decode_output_file_size( DecodeInfo *decInfo );

/* Function to decode size from image data */
long decode_size_from_image( char *buffer);

/* Decode data from image */
char decode_data_from_image( char *buffer);

/* Decode and write to output file */
Status decode_secret_file_data( DecodeInfo *decInfo );

#endif
