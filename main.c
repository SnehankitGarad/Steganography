#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    // -------- Basic argument validation -------- //
    if (argc < 2)
    {
        printf("ERROR: Missing operation type\n");
        printf("Usage:\n");
        printf("  Encoding: ./a.out -e <src.bmp> <secret.txt> [stego.bmp]\n");
        printf("  Decoding: ./a.out -d <stego.bmp> [output_file]\n");
        return 1;
    }

    // -------- Determine encode or decode -------- //
    OperationType op_type = check_operation_type(argv);

    if (op_type == e_encode)
    {
        EncodeInfo encInfo;

        if (argc >= 4 && argc <= 5)
        {
            if (read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                if (do_encoding(&encInfo) == e_success)
                {
                    printf("INFO: Encoding Done Successfully\n");
                }
                else
                {
                    printf("ERROR: Encoding Failed\n");
                }
            }
            else
            {
                printf("ERROR: Encoding Validation Failed\n");
            }
        }
        else
        {
            printf("ERROR: Invalid number of arguments for encoding!\n");
            printf("Usage: ./a.out -e <src.bmp> <secret.txt> [stego.bmp]\n");
        }
    }

    else if (op_type == e_decode)
    {
        DecodeInfo decInfo;

        if (argc >= 3 && argc <= 4)
        {
            if (read_and_validate_decode_args(argc, argv, &decInfo) == e_success)
            {
                if (do_decoding(&decInfo) == e_success)
                {
                    printf("INFO: Decoding Done Successfully\n");
                }
                else
                {
                    printf("ERROR: Decoding Failed\n");
                }
            }
            else
            {
                printf("ERROR: Decoding Validation Failed\n");
            }
        }
        else
        {
            printf("ERROR: Invalid number of arguments for decoding!\n");
            printf("Usage: ./a.out -d <stego.bmp> [output_file]\n");
        }
    }

    else
    {
        printf("ERROR: Unsupported Operation %s\n", argv[1]);
        printf("Use -e for encoding or -d for decoding.\n");
    }

    return 0;
}
