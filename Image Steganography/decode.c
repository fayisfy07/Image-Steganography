#include<stdio.h>
#include<string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

/* Function Definition*/

Status open_file_decode( DecodeInfo *decInfo )
{
	//Opening the source file
	decInfo->fptr_src_image = fopen( decInfo->src_image_fname, "r");
	//do error handling
	if( decInfo->fptr_src_image == NULL )
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_image_fname);

		return e_failure;
	}
	printf("INFO: Opened %s\n", decInfo->src_image_fname);

	return e_success;
}

Status read_and_validate_decode_args( char *argv[], DecodeInfo *decInfo)
{
	//To check if source file is a .bmp file
	//And to check if output file name is given
	if( argv[3] != NULL )
	{
		strcpy(decInfo->output_fname, argv[3]);
	}
	else
	{
		decInfo->output_fname[0]=0;
	}
	if( strstr( argv[2], ".bmp") == NULL )
	{
		printf("ERROR: The source file should be a .bmp file\n");
		return e_failure;
	}
	else
	{
		decInfo->src_image_fname = argv[2];
		return e_success;
	}
}

Status do_decoding( DecodeInfo *decInfo )
{
	printf("INFO: ## Decoding Procedure Started ##\n");
	printf("INFO: Opening required files\n");
	if( open_file_decode( decInfo ) == e_success )
	{
		printf("INFO: Done\n");
		if( skip_bmp_header( decInfo->fptr_src_image ) == e_success )
		{
			printf("INFO: Decoding Magic String Signature\n");
			if( decode_magic_string( decInfo, MAGIC_STRING ) == e_success )
			{
				printf("INFO: Done\n");
				printf("Decoding Output File Extension\n");
				if( decode_output_file_extn_size( decInfo) == e_success )
				{
					if( decode_output_file_extn( decInfo->extn_secret_file , decInfo) == e_success )
					{
						printf("INFO: Done\n");
						if( create_and_open_output_file( decInfo ) == e_success )
						{
							printf("INFO: Done. Opened all required files\n");
							printf("INFO: Decoding %s File Size\n", decInfo->output_fname);
							if( decode_output_file_size( decInfo ) == e_success )
							{
								printf("INFO: Done\n");
								printf("INFO: Decoding %s File Data\n", decInfo->output_fname);
								if( decode_secret_file_data( decInfo ) == e_failure )
								{
									printf("ERROR: Unable to decode secret file data\n");
									return e_failure;
								}
								else
								{
									printf("INFO: Done\n");
									return e_success;
								}
							}
							else
							{
								printf("ERROR: Unable to obtain output file size\n");
								return e_failure;
							}
						}
						else
						{
							printf("ERROR: Unable to open output file\n");
							return e_failure;
						}
					}
					else
					{
						printf("ERROR: Unable to fetch extension of output file\n");
						return e_failure;
					}
				}
				else
				{
					printf("ERROR: Unable to fetch size of output file extension\n");
					return 0;
				}
			}
			else
			{
				printf("ERROR: Invalid Magic String Signature\n");
				return e_failure;
			}
		}
		else
		{
			printf("ERROR: Unable to access %s file\n", decInfo->src_image_fname);
			return e_failure;
		}
	}
	else
	{
		printf("ERROR: Unable to open source file\n");
		return e_failure;
	}
	return e_success;
}

Status skip_bmp_header( FILE *fptr )
{
	/*This fucntion skips the Header data of the image file
	  where no data is encoded */
	fseek( fptr, 54, SEEK_SET);
	if( ftell(fptr) == 54 )
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
}

char decode_data_from_image( char *buffer)
{
	//This function is used to decode data from source image RGB data
	//Obtains bit from LSB of each byte of data in the buffer and forms a character
	char ch=0;
	int k=7, i=0;
	for( i = 0 ; i < 8 ; i++ )
	{
		unsigned char temp = buffer[i] &  1 ;
		ch = ch | ( temp << k );
		k--;
	}
	//printf("char = %c\n", ch);
	return ch;
}

long decode_size_from_image( char *buffer )
{
	//This function fetches size of from RGB data of source image
	//Size stored as long ( 4 byte is obtained and returned)
	long size=0;
	int k=31, i=0;
	for( i = 0 ; i < 32 ; i++ )
	{
		long temp = buffer[i] & ( 1 << 0 );
		size = size | ( temp << k );
		k--;
	}
	return size;
}

Status decode_magic_string( DecodeInfo *decInfo, const char *magic_string )
{
	//Decodes the data from image and obtains the magic string
	/*The function also checks if the obtained magic string is same as the 
	  original magic string signature*/
	int i;
	char buffer[8], ms[3];
	for( i = 0 ; i < strlen(magic_string); i++ )
	{
		if( fread( buffer, 1, 8, decInfo->fptr_src_image) != 8 )
		{
			//Reads 8 byte data from image to obtain 1 byte of output data
			return e_failure;
		}
		ms[i]=decode_data_from_image( buffer); //Stores the magic string character
	}
	ms[i]='\0';

	if( strcmp(	ms, magic_string) != 0 )
	{
		//Checks if the magic string signatures are matching
		return e_failure;
	}
	return e_success;
}

Status decode_output_file_extn_size( DecodeInfo *decInfo)
{
	/* Function reads 32 byte of image data to decode and hence obtain the 
	   size of the extension of the output file */
	char buffer[32];
	if( fread(buffer, 1, 32, decInfo->fptr_src_image) != 32 )
	{
		return e_failure;
	}
	decInfo->output_file_extn_size = decode_size_from_image( buffer);
	return e_success;
}

Status decode_output_file_extn( char *extn , DecodeInfo *decInfo)
{
	/* This function obtains the type of extension of the output file
	   from the source file */
	int i;
	char buffer[8];
	for( i = 0 ; i < (int)(decInfo->output_file_extn_size) ; i++ )
	{
		if( fread(buffer, 1, 8, decInfo->fptr_src_image) != 8 )
		{
			return e_failure;
		}
		extn[i]=decode_data_from_image( buffer );
	}
	return e_success;
}

Status create_and_open_output_file( DecodeInfo *decInfo )
{
	/* Function sets the name of the output file, by adding extension obtained
	   and opens the file with the name */
	//	if( decInfo->output_fname != NULL )
	if( decInfo->output_fname[0] != 0 )
	{
		/*Opens file with user defined file name */
		if( strchr( decInfo->output_fname, '.') != NULL )
		{
			/* Extractes file name, if user entered extension*/
			strcpy( decInfo->output_fname, strtok(decInfo->output_fname, "."));
		}
	}
	else
	{
		/* Opens file with default name, in case of no user input */
		printf("INFO: Output file not mentioned\n");
		strcpy( decInfo->output_fname, "decoded");
	}
		//Appends decoded output file extension to file name and opens the file
		strcat( decInfo->output_fname, decInfo->extn_secret_file);
		decInfo->fptr_output = fopen( decInfo->output_fname, "w");
		if( decInfo->fptr_output != NULL )
		{
			printf("INFO: Opened %s\n", decInfo->output_fname);
			return e_success;
		}
		else
		{
			return e_failure;
		}
}

Status decode_output_file_size( DecodeInfo *decInfo )
{
	/* Function obtains the size of the output file */
	/* This determines the amount of data to be fetched from source image */
	char buffer[32];
	if( fread(buffer, 1, 32, decInfo->fptr_src_image) != 32 )
	{
		return e_failure;
	}
	decInfo->size_output_file = decode_size_from_image( buffer );
	return e_success;
}

Status decode_secret_file_data( DecodeInfo *decInfo )
{
	/* Function obtains 8 byte of data from source image and decodes it
	   and writes it to the output file*/
	unsigned char ch;
	char buffer[8];
	for( int  i = 0 ; i < (int)(decInfo->size_output_file) ; i++ )
	{
		if( fread(buffer, 1, 8, decInfo->fptr_src_image ) != 8 )
		{
			return e_failure;
		}
		ch = decode_data_from_image( buffer );
		if( fwrite( &ch, 1, 1, decInfo->fptr_output ) != 1 )
		{
			return e_failure;
		}
	}
	return e_success;
}
