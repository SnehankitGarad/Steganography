#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"    
#include "common.h"

typedef struct _DecodeInfo
{
    // --- Input and output file pointers ---
    FILE *fptr_stego_image;   // Encoded image input
    FILE *fptr_secret_file;   // Decoded output file

    // --- File names ---
    char *stego_image_fname;   // Name of stego image file -->.bmp
    char *output_fname;        // Output file -->decoded secret

    // --- Decoding metadata ---
    uint secret_file_size;  

} DecodeInfo;

/* 1. Validate decode args (argv -> DecodeInfo fields) */
Status read_and_validate_decode_args(int argc,char *argv[],DecodeInfo *decodeinfo);

/* 2. Main decoding */
Status do_decoding(DecodeInfo *decInfo);

/* 3. Open image / output files needed for decoding */
Status open_img_files(DecodeInfo *decInfo);


/* 4. Read and Skip BMP header  */
Status skip_bmp_header(FILE *fptr_image);

/* 5. Decode magic string  */
Status decode_magic_string(DecodeInfo *decInfo);

/* 6. Decode the extension length -->32 bits */
Status decode_secret_file_extn_size(int *extn_len, DecodeInfo *decInfo);

/* 7. Decode the extension string itself -->extn_len bytes */
Status decode_secret_file_extn(char *extn, int extn_len, DecodeInfo *decInfo);

/* 8. Decode file size  */
Status decode_secret_file_size(uint *file_size, DecodeInfo *decInfo);

/* 9. Decode the secret file data & write to secret output */
Status decode_secret_file_data(DecodeInfo *decInfo);

/* 10. extract one data byte from 8 LSBs */
Status decode_byte_from_lsb(unsigned char *out_byte, unsigned char image_buffer[]);

/*11. reverse process of -->encode_int_to_lsb() function used in encoding.*/
Status decode_int_from_lsb(char *image_buffer, int *data);


#endif 