#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>
#include <stdbool.h>
#include "dsvParser.h"






int main()
{
    char* filepath = "./test.csv";
    /* can be anything other than " or \n */
    char delim = ',';

    /* READ A CSV, escaping is supported */
    Parsed parsed_csv = dsvParseFile(filepath, delim);

    /* failed to parse! Always check is valid */
    if (!parsed_csv.valid) {
        fprintf(stderr,"failed to parse csv\n");
    }

    /* inbuilt print function */
    printParsed(parsed_csv);

    /* write to a fie, you can choose the path and delimiter. Will always ""
     * wrap each value*/
    int result = dsvWriteFile(parsed_csv,"./test1.csv",';');
    if (result) {
        fprintf(stderr,"failed to write to file\n");
    }


    /* access elements directly */
    for (size_t i = 0; i<parsed_csv.rows; i++) {
        for (size_t j = 0; j<parsed_csv.cols; j++) {
            printf("%s ",parsed_csv.content[i][j]);
        }
        printf("\n");
    }
    
    /* always free */
    dsvFreeParsed(parsed_csv);


    return EXIT_SUCCESS;
}

