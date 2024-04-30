#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>
#include <stdbool.h>
#include "dsvParser.h"






int main()
{
    char* filepath = "./test.csv";
    char delim = ',';

    Parsed parsed_csv = dsvParseFile(filepath, delim);

    printf("DEBUG\n");
    printParsed(parsed_csv);
    int result = dsvWriteFile(parsed_csv,"./test1.csv",';');
    if (result) {
        fprintf(stderr,"failed to write to file\n");
    }
    dsvFreeParsed(parsed_csv);


    return EXIT_SUCCESS;
}

