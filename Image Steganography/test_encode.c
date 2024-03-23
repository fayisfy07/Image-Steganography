#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

/* Documantation
Nmae: Mohammed Fayis
Batch: 23029_C
Project Name: LSB Steganography
Date of Submission: 28/2/2024
 */

int main(int argc, char *argv[])
{
	EncodeInfo encInfo;
	DecodeInfo decInfo;
	//check_operation function
	int ret;
	if (argc >=2)
	{
		ret = check_operation_type(argv);
	}
	else
	{
		printf("Insufficient number of command line arguments\n");
		return 0;
	}

	if (ret == e_encode)
	{
		printf("Selected encoding\n");

		if(argc >=4) //to check sufficient number if cla is passed
		{
			//read and validate function
			if(read_and_validate_encode_args(argv, &encInfo)== e_success)
			{
				printf("Started Encoding\n");
				//do_encoding function
				if(do_encoding(&encInfo) == e_success)
				{
					printf("Encoding Done Successfully\n");
				}
				else
				{
					printf("ERROR: Encoding Unsuccessful\n");
				}
			}
		}
		else
		{
			printf("ERROR: Insuffient number of Command Line Argument\n");
		}

	}
	else if(ret == e_decode)
	{
		printf("Selected decoding\n");
		if (argc >=3)
		{

			if (read_and_validate_decode_args (argv, &decInfo) == e_success )
			{
				if (do_decoding (&decInfo) == e_success)
				{
					printf ("Decoding Done Successfully\n");
				}
				else
				{
					printf("ERROR: Decoding Unsuccessfl\n");
				}
			}
		}
	}
	else
		printf("ERROR: Unssupported\n");

	return 0;
}

OperationType check_operation_type(char *argv[])
{
	if(strcmp(argv[1], "-e") == 0)		//returing 0
		return e_encode;
	else if (strcmp (argv[1], "-d") == 0)
		return e_decode;
	else
		return e_unsupported;
}

