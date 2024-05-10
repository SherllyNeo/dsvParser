#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>
#include <stdbool.h>
#include <string.h>
#include "dsvParser.h"






int main()
{


    char* filepath = "./test.csv";
    /* can be anything other than " or \n */
    char delim = ',';

    /* READ A CSV, escaping is supported */
    DSV parsed_csv = dsvParseFile(filepath, delim);

    /* failed to parse! Always check is valid */
    if (!parsed_csv.valid) {
        fprintf(stderr,"failed to parse csv\n");
    }

    /* insert a row - must be malloc'd so the row can be freed later. strdup will malloc for us. 
     * Be careful, here we insert 5 into a csv of width 4, the insert function will get rid of element 5 "five" and turn the excess elements to null (and null terminate), 
     * effectively silently trunacting and MUST be a null terminated array */

     char** row = (char**)malloc(sizeof(char*) * 4);
    if (!row) {
        fprintf(stderr, "ERROR: Unable to allocate memory for row\n");
        return 1;
    }
    row[0] = strdup("one");
    row[1] = strdup("two");
    row[2] = strdup("threessssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss");
    row[3] = strdup("four");
    row[4] = strdup("five");
    row[5] = NULL;
    
    size_t insert_index = 1; /* ensure not out of bounds */

    int result_insert = dsvInsertRow(&parsed_csv,row,insert_index);
    if (result_insert) {
        fprintf(stderr,"failed to add new row\n");
    }


    /* inbuilt print function */
    dsvPrintDSV(parsed_csv);

    /* write to a fie, you can choose the path and delimiter. Will always ""
     * wrap each value*/
    int result_write = dsvWriteFile(parsed_csv,"./test1.csv",';');
    if (result_write) {
        fprintf(stderr,"failed to write to file\n");
    }

    /* remove a row */
    int result_remove = dsvRemoveRow(&parsed_csv,0);
    if (result_remove) {
        fprintf(stderr,"failed to remove\n");
    }

    printf("\n\n");
    /* access elements directly */
    for (size_t i = 0; i<parsed_csv.rows; i++) {
        for (size_t j = 0; j<parsed_csv.cols; j++) {
            printf("%s ",parsed_csv.content[i][j]);
        }
        printf("\n");
    }

    /* always free */
    int freed = dsvFreeDSV(parsed_csv);
    if (freed) {
        fprintf(stderr,"failed to free\n");
    }


    return EXIT_SUCCESS;
}

