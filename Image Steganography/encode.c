#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */

uint get_image_size_for_bmp(FILE *fptr_image)
{
	uint width, height;		//from the 18th to next 4 byte are long data
	// Seek to 18th byte
	fseek(fptr_image, 18, SEEK_SET);

	// Read the width (an int)
	fread(&width, sizeof(int), 1, fptr_image);	//width of RGB Data in pixels (1 pixels means 3 bytes(RGB))
	printf("width = %u\n", width);

	// Read the height (an int)
	fread(&height, sizeof(int), 1, fptr_image);	//height of RGB Data
	printf("height = %u\n", height);

	// Return image capacity
	return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status open_files(EncodeInfo *encInfo)
{
	// Src Image file
	encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
	// Do Error handling
	if (encInfo->fptr_src_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
		return e_failure;
	}
	printf("INFO: Opened %s\n", encInfo->src_image_fname);

	// Secret file
	encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
	// Do Error handling
	if (encInfo->fptr_secret == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

		return e_failure;
	}
	printf("INFO : opened %s\n", encInfo->secret_fname);

	// Stego Image file
	encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
	// Do Error handling
	if (encInfo->fptr_stego_image == NULL)
	{
		perror("fopen");
		fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

		return e_failure;
	}

	printf("INFO : opened %s\n", encInfo->secret_fname);

	return e_success;
}


Status read_and_validate_encode_args (char *argv[], EncodeInfo *encinfo)
{
	//to check the source file is .bmp
	if (strstr (argv[2], ".bmp") != NULL)
	{
		//store filename to structure
		encinfo->src_image_fname = argv[2];
	}
	else
	{
		printf("Error : The source file passed should a .bmp file\n");
	}
	if(strchr (argv[3], '.') != NULL)
	{
		encinfo->secret_fname = argv[3];
		//after sorting the extn of secret file and store to structure
		strcpy(encinfo->extn_secret_file, strchr(argv[3],'.'));
	}
	else
	{
		printf("Error : The secret file does not have an extention\n");
		return e_failure;
	}
	// check if output filename is passed or not
	if(argv[4] != NULL)
	{
		//check if it is a .bmp file
		if(strstr (argv[4], ".bmp") != NULL)
		{
			encinfo->stego_image_fname = argv[4];
		}
		else
		{
			printf("Error : The output file is not a bmp file\n");
			return e_failure;
		}
	}
	//else give error and use defualt name for stroing output file name, store defualt filename if not .bmp
	else
	{
		printf("INFO: Output file not mentioned. Creating stego.bmp as default\n");
		encinfo-> stego_image_fname = "stego.bmp"; 		//output file name is not mentioned, so creating stego.bmp as defult file
	}

	return e_success;
}

Status do_encoding( EncodeInfo *encInfo)
{
	printf("INFO: Opening required files\n");
	if(open_files(encInfo) == e_success)
	{
		printf("INFO: Done\n");
	   	printf("INFO: ## Encoding procedure started ##\n");
		printf("checking for %s size\n", encInfo->secret_fname);
		if(get_file_size (encInfo->fptr_secret) >0)
		{
			printf("INFO: Done. Not Empty\n");
			printf("checking for %s capacity to handle %s\n", encInfo->src_image_fname, encInfo->secret_fname);

			if(check_capacity(encInfo) == e_success)
			{
				printf("INFO: Done\n");
				printf("INFO: Copying image header\n");

				if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image ) == e_success)
				{
					printf("INFO: Done\n");
					printf("INFO: Encoding Magic String Signature\n");

					if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
					{
						printf("INFO: Done\n");
						printf("INFO: Encoding Extension Size %s\n", encInfo->extn_secret_file);
						if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo) == e_success)
						{
							printf("INFO: Done\n");
							printf("INFO: Encoding %s Extension\n", encInfo->extn_secret_file);
							if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
							{
								printf("INFO: Done\n");
								printf("INFO: Encoding %s File Size\n", encInfo->extn_secret_file);
								if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
								{
									printf("INFO: Done\n");
									printf("INFO: Encoding %s File Data\n", encInfo->extn_secret_file);

									if(encode_secret_file_data(encInfo) == e_success)
									{
										printf("INFO: Done\n");
										printf("INFO: Copying Left Over Data\n");

										if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo-> fptr_stego_image) == e_failure)
										{
											return e_failure;
										}
										else
										{
											printf("INFO: DONE\n");
											return e_success;
										}
									}

									else
									{
										return e_failure;
									}

								}

								else
								{
									return e_failure;
								}

							}

							else
							{
								return e_failure;
							}

						}
						else
						{
							return e_failure;
						}
					}
					else
					{
						return e_failure;
					}
				}
				else
				{
					return e_failure;
				}
			}
			else
			{
				return e_failure;
			}
		}
		else
		{
			return e_failure;

		}
	}
	else
	{
		return e_failure;
	}	

	return e_success;
}


//function to check if the source image file has enough space to store data to be encoded
Status check_capacity (EncodeInfo *encInfo)
{		//find the bmp file size
	encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
	encInfo-> size_secret_file = get_file_size(encInfo->fptr_secret);
	if ( (encInfo->image_capacity) > ((strlen(MAGIC_STRING) + strlen(encInfo->extn_secret_file) + 4 + 4 + encInfo->size_secret_file) * 8) )
	{
		return e_success;
	}
	else
	{
		return e_failure;
	}
}

uint get_file_size(FILE *fptr)
{
	//function to get the number of characters in the secret file
	fseek(fptr, 0, SEEK_END);	//moves to the end of the file
	return ftell(fptr);		//returns the number of index
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
	//read 54 byte header data from source file and write to stego image file 
	rewind(fptr_src_image);	//source image file pointer should be rest after get image file size function is called

	char buffer[54];	//temp to store of 54 bytes
	if(fread (buffer, 1, 54, fptr_src_image) != 54)
	{
		return e_failure;
	}
	if (fwrite (buffer,1, 54, fptr_dest_image) !=54)	//stores the data obtained from source file to destination file
	{
		return e_failure;
	}
	return e_success;
}

//encode the magic string data to the output image file
//instead of using the loops, encode_data_to_image function is called to write the data to the image
Status encode_magic_string (const char *magic_string, EncodeInfo *encInfo)
{
	if( encode_data_to_image(magic_string, strlen(magic_string), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure )
	{
		return e_failure;
	}

	return e_success;
}


Status encode_data_to_image( const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{

	//use a loop for size times and call the following funcions to incorporate magic string data to the destination file
	//store 8 bytes of RGB data from source file to buffer

	char buffer[8];		//charater buffer 8 byte RGB data
	for(int i=0; i<size; i++)
	{
		//to get 8 bytes of data from source file and store it in the buffer to store 1 byte of magic string data
		if( fread (buffer, 1, 8, fptr_src_image) != 8 )
		{
			return e_failure;
		}
		if ( encode_byte_to_lsb (data[i], buffer) == e_failure)		//8 bytes of magic string character data will write to buffer
		{
			return e_failure;
		}
		if (fwrite (buffer, 1, 8, fptr_stego_image) != 8)
		{	//to write data obtained to the output file 
			return e_failure;
		}

	}

	return e_success;

}



Status encode_byte_to_lsb(char data, char *image_buffer)
{
	//loop is used to store each bit of the 1 byte data to the LSB of each character of buffer
	int i=0, k=0;
	for(i=7; i>=0; i--)
	{
		unsigned char temp = (data & ( 1 << i ) );
		temp = temp >> i;
		image_buffer[k] = (image_buffer[k] & (~ (0x01) ) ) | temp;		//A pointer to the buffer containing the image data where the byte will be encoded
		k++;
	}
	return e_success;
}


Status encode_size_to_lsb(long size, char *buffer)
{
	//function to encode the size data (4 byte) to the buffer of size 32 byte
	//run loop to store each bit of size to each element of buffer

	int i=0, k=0;
	for(i=31; i>=0; i--)
	{
		unsigned char temp = (size & (1 << i));
		temp = temp >>i;
		buffer[k] = (buffer [k] & (~ (0x01)) ) | temp;
		k++;
	}
	return e_success;

}

//function to encode the extn size of secret file to the output image
Status encode_secret_file_extn_size (long secret_file_extn_size, EncodeInfo *encInfo)
{
	char buffer[32];	//int 4 byte 32 bit of ext
	//fread -> read 32 byte -> (from source image)
	if ( fread (buffer, 1, 32, encInfo->fptr_src_image) != 32 )
	{
		return e_failure;
	}
	if (encode_size_to_lsb (secret_file_extn_size, buffer) == e_failure)
	{
		return e_failure;
	}

	if ( fwrite (buffer, 1, 32, encInfo->fptr_stego_image) != 32)
	{
		return e_failure;
	}
	return e_success;
}

//function encode the extension of the secret file to the output image
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo )
{

	for(int i=0 ; i< strlen(file_extn) ; i++)
	{
		//if fread doesnot read 8 chars from source image, process failed
		if (fread ( encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8)
		{
			return e_failure;
		}
		if ( encode_byte_to_lsb (file_extn[i], encInfo->image_data) == e_failure)
		{
			return e_failure;
		}

		if (fwrite(encInfo->image_data, 1, 8,encInfo->fptr_stego_image) != 8 )	// return data obtained to the output file 
		{
			return e_failure;
		}
	}
	return e_success;
}

//<<-----encode_secret_file_size------>>//
Status encode_secret_file_size (long file_size, EncodeInfo *encInfo)
{
	char buffer[32];
	if( fread (buffer,1, 32, encInfo->fptr_src_image) != 32)
	{
		//fread -> read 32 byte -> from source image
		return e_failure;
	}
	if (encode_size_to_lsb (file_size, buffer) == e_failure)
	{
		return e_failure;
	}

	if (fwrite (buffer,1 , 32, encInfo->fptr_stego_image) !=32)
	{
		//fwrite -> write 32 byte to the (destination image), ie encoded file
		//return success image
		return e_failure;
	}

	return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{
	//to brought data from secret.txt file 
	char secret_data[encInfo->size_secret_file];
	rewind(encInfo->fptr_secret);
	fread(secret_data, 1, encInfo->size_secret_file, encInfo->fptr_secret);

	//loop to encode each byte to the RGB data
	for(int i=0; i< encInfo-> size_secret_file; i++)
	{
		if (fread (&encInfo-> image_data, 1, 8, encInfo->fptr_src_image) != 8)		//to get RGB data from BMP file
		{
			return e_failure;
		}
		if (encode_byte_to_lsb(secret_data[i], encInfo->image_data) == e_failure)//to store each character to the bmp file buffer
		{
			return e_failure;
		}
		if (fwrite(&encInfo-> image_data, 1, 8, encInfo->fptr_stego_image) != 8)		// to write the encoded buffer to the output file
		{
			return e_failure;
			//if fwrite does not successfully write to output file from buffer and return 8 (no. of characters written to file), then it will fail.
		}
	}

	return e_success;

}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
	//use buffer to read rest of data from source file and write to the output file using a loop
	char ch;	//read each byte from the file and store to the variable

	while (fread (&ch, 1, 1, fptr_src) != 0) 	//use while loop untill return fread  returns 0
	{
		if (fwrite (&ch, 1, 1, fptr_dest) != 1)
		{
			// if character is not successfully read to ch, the process fails.
			return e_failure;
		}
	}
	return e_success;
}















