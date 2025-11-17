#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);

    // Return image capacity
    return width * height * 3;
}

/* ------------------------- get_file_size (added) ---------------------------- */
uint get_file_size(FILE *fptr)
{
    long curr = ftell(fptr);
    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    fseek(fptr, curr, SEEK_SET);
    return size;
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
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
        return e_failure;
    }

    return e_success;
}

//------------------------------------------- Operation Type-----------------------------------------------------------------------
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1] , "-e")==0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1],"-d")==0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

//---------------------------------------- Read And Validate Encode --------------------------------------------------------------
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *p = strstr(argv[2], ".bmp");

    if (p != NULL)
    {
        if (strcmp(p, ".bmp") == 0)
        {
            encInfo->src_image_fname = argv[2];
        }
        else
        {
            printf("Invalid first\n");
            return e_failure;
        }
    }

    char* dot = strstr(argv[3], ".");

    if(dot != NULL)
    {
        if((strcmp(dot,".txt")==0) || (strcmp(dot,".c")==0) || (strcmp(dot,".sh")==0))
        {
            encInfo->secret_fname = argv[3];

            strcpy(encInfo->extn_secret_file, dot + 1);
        }
        else
        {
            printf("Invalid!!\n");
            return e_failure;
        }
    }

    char *final = NULL;

    if (argv[4] != NULL)
        final = strstr(argv[4], ".bmp");

    if (final != NULL && strcmp(final, ".bmp") == 0)
        encInfo->stego_image_fname = argv[4];
    else
        encInfo->stego_image_fname = "stego.bmp";

    return e_success;
}

//------------------------------------- Status Do Encoding ------------------------------------------------------------------------
Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)
    {
        if(check_capacity(encInfo) == e_success)
        {
            if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("%ld\n",ftell(encInfo->fptr_stego_image));
                if(encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("%ld\n",ftell(encInfo->fptr_stego_image));
                    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("%ld\n",ftell(encInfo->fptr_stego_image));
                        if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("%ld\n",ftell(encInfo->fptr_stego_image));
                            int secret_file_size = get_file_size(encInfo->fptr_secret);
                            encInfo->secret_file_size = secret_file_size;

                            if(encode_secret_file_size(secret_file_size, encInfo) == e_success)
                            {
                                printf("%ld\n",ftell(encInfo->fptr_stego_image));
                                if(encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("%ld\n",ftell(encInfo->fptr_stego_image));
                                    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
                                    {

                                        return e_success;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return e_failure;
}

//----------------------------------------Check Capacity----------------------------------------------------------------------------
Status check_capacity(EncodeInfo *encInfo)
{
    int image_capacity =get_image_size_for_bmp(encInfo->fptr_src_image);

    int magic_len = strlen(MAGIC_STRING);
    int Extension_len = strlen(encInfo-> extn_secret_file);
    int secret_file_size = get_file_size(encInfo->fptr_secret);

    int Encoding_thing =54 + (magic_len + 4 + Extension_len + 4 + secret_file_size) * 8;

    if(Encoding_thing < image_capacity)
    {
        return e_success;
    }
    else
    {
       return e_failure;
    }
}

//-----------------------------------Copy BMP Header------------------------------------------------------------------------------
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    fseek(fptr_src_image, 0, SEEK_SET);

    char buffer[54];
    fread(buffer,sizeof(char),54,fptr_src_image);
    fwrite(buffer,sizeof(char),54,fptr_dest_image);

    return e_success;
}

//------------------------------------Encode Magic String---------------------------------------------------------------------------
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    if(encode_data_to_image((char*)magic_string, strlen(magic_string),encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
    {
        return e_success;
    }
    return e_failure;
}

//-----------------------------------encode_data_to_image-------------------------------------------------------------------------
Status encode_data_to_image(char *data, int size, FILE* fptr_src_image,FILE* fptr_stego_image )
{
    unsigned char buffer[8];
    for (int i = 0; i < size; i++)
    {
        fread(buffer, sizeof(char), 8, fptr_src_image);
        encode_byte_to_lsb(data[i], buffer);
        fwrite(buffer, sizeof(char), 8,fptr_stego_image);
    }
    return e_success;
}

Status encode_byte_to_lsb(char data,unsigned char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        unsigned char bit = (data >> (7 - i)) & 1;

        image_buffer[i] = image_buffer[i] & ~1;

        image_buffer[i] = image_buffer[i] | bit;
    }
    return e_success;
}

//-----------------------------Encode Secret File Extn Size --------------------------------------------------------------------
Status encode_secret_file_extn_size(int size,FILE* fptr_src_image,FILE* fptr_dest_image)
{
    unsigned char buffer[32];

    fread(buffer, sizeof(char), 32, fptr_src_image);

    encode_int_to_lsb(size, buffer);

    fwrite(buffer, sizeof(char), 32, fptr_dest_image);

    return e_success;
}

Status encode_int_to_lsb(int data,unsigned char *image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        unsigned char bit = (data >> (31 - i)) & 1;

        image_buffer[i] = image_buffer[i] & ~1;

        image_buffer[i] = image_buffer[i] | bit;
    }
    return e_success;
}

//-----------------------------------Encode secret file extension ------------------------------------------------------------
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char extn_padded[4] = {0};
    strncpy(extn_padded, file_extn, 4);

    if(encode_data_to_image(extn_padded, 4, encInfo->fptr_src_image, encInfo->fptr_stego_image)==e_success)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

//---------------------------------------Encode secret file data-------------------------------------------------------------------
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    unsigned char size[32];
    fread(size,sizeof(char),32,encInfo->fptr_src_image);

    encode_int_to_lsb(file_size, size);

    fwrite(size,sizeof(char),32,encInfo->fptr_stego_image);

    return e_success;
}

//---------------------------------------Encode secret file data-------------------------------------------------------------------
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char secret_file_data[encInfo->secret_file_size];

    fread(secret_file_data, sizeof(char),encInfo->secret_file_size, encInfo->fptr_secret);

    if (encode_data_to_image(secret_file_data,encInfo->secret_file_size,encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
    {
        return e_success;
    }

    return e_failure;
}

//-----------------------------------Copy Remaining img data----------------------------------------------------------------------
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }

    return e_success;
}






