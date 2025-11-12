#include <stdio.h>
#include "encode.h"
#include "types.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;

    if (check_operation_type(argv) == e_encode)
    {
        if (argc >= 4 && argc <= 5)      
        {
            if (read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                // Encoding process
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
                printf("ERROR: Validation Failed\n");
            }
        }
        else
        {
            printf("\nERROR: Invalid number of arguments\n");
        }
    }
    else
    {
        printf("ERROR: Unsupported Operation\n");
    }

    return 0;
}
