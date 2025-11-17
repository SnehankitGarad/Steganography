#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

#define EXT_DATA_BYTES 4  // extension stored as 4 data bytes

Status open_img_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");    // open stego file read-binary
    if (decInfo->fptr_stego_image == NULL)                                 // check open failed
    {
        printf("Error: Unable to open stego image file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    decInfo->fptr_secret_file = fopen(decInfo->output_fname, "wb");       // open output file write-binary
    if (decInfo->fptr_secret_file == NULL) 
    {
        printf("Error: Unable to create output file %s\n", decInfo->output_fname);
        fclose(decInfo->fptr_stego_image);
        return e_failure;
    }

    return e_success;
}

Status read_and_validate_decode_args(int argc, char *argv[], DecodeInfo *decInfo)
{
    if (argc < 3) // need at least option + file
    {
        printf("Error: Missing arguments!\n");
        printf("Usage: ./a.out -d <stego_image.bmp> [output_file]\n");
        return e_failure;
    }

    int len = strlen(argv[2]);                                  // length of stego filename
    if (len < 4 || strcmp(argv[2] + (len - 4), ".bmp") != 0)    // check .bmp extension
    {
        printf("Error: %s is not a valid .bmp file!\n", argv[2]);
        return e_failure;
    }

    /* ---- store filenames into struct ---- */
    decInfo->stego_image_fname = argv[2]; // save stego filename

    if (argc > 3)  // if output file name provided
    {
        char *ptr = strstr(argv[3], ".");
        if (ptr == NULL)
        {
            printf("Error: Output file %s has no extension!\n", argv[3]);
            return e_failure;
        }

        decInfo->output_fname = argv[3];  // save output filename
    }
    else
    {
        decInfo->output_fname = "decoded_secret.txt";  // default  output file name
    }

    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    int extn_len;   //decoded extn length
    uint file_size; // decoded file size

      printf("\nStarting Decoding Process...\n");

    if (open_img_files(decInfo) != e_success)
        return e_failure;
    printf("Done: Opened stego imagefile\n");

    if (skip_bmp_header(decInfo->fptr_stego_image) != e_success)
        return e_failure;
     printf("Done: Skipped 54-byte BMP Header\n");

    if (decode_magic_string(decInfo) != e_success)
    {
        printf("Magic String Mismatch --> Not a valid stego file!\n");
        return e_failure;
    }
    printf("Done: Magic String Verified Successfully\n");

    if (decode_secret_file_extn_size(&extn_len, decInfo) != e_success)
        return e_failure;
    printf("Done: Extension Length Decoded --> %d\n", extn_len);

    char extension[20];
    if (decode_secret_file_extn(extension, extn_len, decInfo) != e_success)
        return e_failure;
    extension[extn_len] = '\0';
    //printf("Extension Decoded --> %s\n", extension);

    if (decode_secret_file_size(&file_size, decInfo) != e_success)
        return e_failure;
    decInfo->secret_file_size = file_size;
    printf("Done: Secret File Size = %u bytes\n", file_size);

    decInfo->secret_file_size = file_size;

    if (decode_secret_file_data(decInfo) != e_success)
        return e_failure;
    printf("Done: Secret File Data Extracted Successfully\n");

    printf("====================================\n");
    printf(" Decoding Completed Successfully!\n");
    printf(" Output saved in: %s\n", decInfo->output_fname);
    printf("====================================\n");


    return e_success;
}

Status skip_bmp_header(FILE *fptr_image)
{
    fseek(fptr_image, 54, SEEK_SET);  // jump to byte 54
    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    char magic_str[3];              // buffer for magic string chars
    unsigned char img_buffer[8];     // buffer for 8 carrier bytes
    int size = strlen(MAGIC_STRING);

    for (int i = 0; i < size; i++)      // for each magic char
    {
        for (int j = 0; j < 8; j++)      // read 8 bytes from image
        {
            int data = fgetc(decInfo->fptr_stego_image);    // get one image byte
            if (data == EOF) 
                return e_failure;
            img_buffer[j] = (unsigned char)data;      // store byte
        }
        decode_byte_from_lsb((unsigned char *)&magic_str[i], img_buffer); // decode one char
    }
    magic_str[2] = '\0';

    if (!strcmp(magic_str, MAGIC_STRING))  // compare with expected magic string
        return e_success;
    else
        return e_failure;
}

Status decode_byte_from_lsb(unsigned char *out_byte, unsigned char image_buffer[])
{
    unsigned char value = 0;
    for (int i = 0; i < 8; i++)
    {
        unsigned char bit = image_buffer[i] & 1;
        value = (value << 1) | bit;
    }
    *out_byte = value;
    return e_success;
}

Status decode_secret_file_extn_size(int *extn_len, DecodeInfo *decInfo)
{
    char buf32[32];                   // buffer for 32 image bytes
    for (int i = 0; i < 32; ++i)     // read 32 bytes
    {
        int c = fgetc(decInfo->fptr_stego_image);
        if (c == EOF) 
            return e_failure;
        buf32[i] = (char)c;       // store
    }

    decode_int_from_lsb(buf32, extn_len);   // convert 32 LSBs --> int

    if (*extn_len <= 0 || *extn_len > EXT_DATA_BYTES) 
    {
        printf("Error: decoded extension length invalid: %d\n", *extn_len);
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_extn(char *extn, int extn_len, DecodeInfo *decInfo)
{
    if (extn_len <= 0) 
        return e_failure;

    for (int i = 0; i < extn_len; i++)
    {
        unsigned char buffer[8];
        for (int j = 0; j < 8; j++) {
            int c = fgetc(decInfo->fptr_stego_image);
            if (c == EOF) 
                return e_failure;
            buffer[j] = (unsigned char)c;  // save as unsigned

        }
        decode_byte_from_lsb((unsigned char *)&extn[i], buffer);
    }

    extn[extn_len] = '\0';
    return e_success;
}

Status decode_secret_file_size(uint *file_size, DecodeInfo *decInfo)
{
    char buf32[32];
    for (int i = 0; i < 32; ++i) 
    {
        int c = fgetc(decInfo->fptr_stego_image);
        if (c == EOF) 
            return e_failure;
        buf32[i] = (char)c;
    }

    int tmp = 0;
    decode_int_from_lsb(buf32, &tmp);
    if (tmp < 0) 
        return e_failure;
    *file_size = (uint)tmp;

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    uint remaining = decInfo->secret_file_size;
    unsigned char image_buf[8];

    while (remaining > 0) {
        for (int j = 0; j < 8; ++j) 
        {
            int c = fgetc(decInfo->fptr_stego_image);
            if (c == EOF) 
                return e_failure;
            image_buf[j] = (unsigned char)c;
        }
        unsigned char out;
        decode_byte_from_lsb(&out, image_buf);
        if (fputc(out, decInfo->fptr_secret_file) == EOF) 
            return e_failure;
        --remaining;
    }

    return e_success;
}

Status decode_int_from_lsb(char *image_buffer, int *data)
{
    int value = 0;
    for (int i = 0; i < 32; i++) 
    {
        unsigned char bit = (unsigned char)image_buffer[i] & 1;
        value = (value << 1) | bit;
    }
    *data = value;
    return e_success;
}
