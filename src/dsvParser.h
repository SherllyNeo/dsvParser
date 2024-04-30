#ifndef PARSER

#define PARSER


#define ERR_FILE 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_source(char* filepath,size_t* size) {

    FILE* fp = fopen(filepath,"r");
    if (!fp) {
        fprintf(stderr,"DSV_ERR: unable to open file %s\n",filepath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size_t end = ftell(fp);
    rewind(fp);

    
    char* source = (char*)malloc(end+1);

    if (!source) {
        fprintf(stderr, "Failed to allocate memory for file content %s\n",filepath);
        fclose(fp);
        return NULL;
    }


    fread(source, 1, end, fp);
    fclose(fp);

    *size = end;

    return source;
}

typedef struct {
    char*** content;
    size_t rows;
    size_t cols;
    bool valid;
} Parsed;

void printParsed(Parsed parsed) {
    printf("rows: %zu, cols: %zu\n",parsed.rows,parsed.cols);
    for (size_t i = 0; i < parsed.rows; i++) {
        for (size_t j = 0; j < parsed.cols; j++) {
            printf("|%s| ",parsed.content[i][j]);
        }
        printf("\n");
    }
}


Parsed parse_source(char* source, size_t size, char delim) {

    size_t cur = 0;

    size_t rows_est = 5;
    size_t cols_est = 6;
    size_t strings_len_est = 100;
    Parsed returnVal = { NULL, 0, 0, false };

    returnVal.content = (char***)malloc(sizeof(char**) * rows_est);
    if (!returnVal.content) {
        fprintf(stderr, "PARSE_ERR: Unable to allocate memory for rows\n");
        return returnVal; // Return an invalid Parsed struct
    }
    for (size_t x = 0; x < rows_est; x++) {
        returnVal.content[x] = (char**)malloc(sizeof(char*) * cols_est); /* over estimate size for cols (and values) */
        if (!returnVal.content[x]) {
            fprintf(stderr, "PARSE_ERR: Unable to allocate memory for cols\n");
            // Free previously allocated memory
            for (size_t i = 0; i < x; i++) {
                for (size_t j = 0; j < cols_est; j++) {
                    free(returnVal.content[i][j]);
                }
                free(returnVal.content[i]);
            }
            free(returnVal.content);
            return returnVal; 
        }
        for (size_t y = 0; y < cols_est; y++) {
            returnVal.content[x][y] = (char*)malloc(sizeof(char) * strings_len_est); /* over estimate size for strings */
            if (!returnVal.content[x][y]) {
                fprintf(stderr, "PARSE_ERR: Unable to allocate memory for strings\n");
                // Free previously allocated memory
                for (size_t i = 0; i <= x; i++) {
                    for (size_t j = 0; j < y; j++) {
                        free(returnVal.content[i][j]);
                    }
                    free(returnVal.content[i]);
                }
                free(returnVal.content);
                return returnVal;
            }
        }
    }

    // At this point, returnVal.content is successfully allocated
    // Your code goes here



    bool valid = false;
    bool escaping = false;
    size_t x = 0;
    size_t y = 0;

    size_t cols = 0;

    for (size_t i = 0; i < size; i++) {
        char current_char = source[i];

        if (current_char == '"') {
            escaping = !escaping;
            }
        else if (current_char == delim && !escaping) {
            returnVal.content[x][y][cur] = '\0';
            y++;
            cur = 0;

        } else if (current_char == '\n') {
            returnVal.content[x][y][cur] = '\0';
            x++;
            cols = (size_t)y;
            y = 0;
            cur = 0;
        } else {
            /* add here */
            returnVal.content[x][y][cur] = current_char;
            cur++;
        }
    }

    valid = true;
    returnVal.valid = valid;
    returnVal.rows = x;
    returnVal.cols = cols + 1;

    return returnVal;
}


/* PUBLIC FUNCTIONS */

void dsvFreeParsed(Parsed parsed) {
    // Don't forget to free the memory when done
    for (size_t i = 0; i < parsed.rows; i++) {
        for (size_t j = 0; j < parsed.cols; j++) {
            free(parsed.content[i][j]);
        }
        free(parsed.content[i]);
    }
    free(parsed.content);
}


Parsed dsvParseFile(char* filepath, char delim) {
    Parsed returnVal = { NULL, 0, 0, false };
    if (delim == '"' || delim == '\n') {
        fprintf(stderr,"DELIM cannot be \" or \\n \n");
        return returnVal;
    }

    size_t size = 0;

    char* source = read_source(filepath,&size);
    if (!source) {
        fprintf(stderr, "failed to read %s\n",filepath);
        exit(ERR_FILE);
    }

    returnVal = parse_source(source,size,delim);
    if (!returnVal.valid) {
        fprintf(stderr, "failed to read %s\n",filepath);
        return returnVal;
    }
    return returnVal;
}

size_t dsvWriteFile(Parsed parsed,char* filepath,char delim) {
    FILE* fp = fopen(filepath,"w");
    if (!fp) {
        fprintf(stderr,"DSV_ERR: unable to open file to write to %s\n",filepath);
    }
    for (size_t i = 0; i < parsed.rows; i++) {
        for (size_t j = 0; j < parsed.cols; j++) {
            fprintf(fp,"\"%s\"",parsed.content[i][j]);
            if (j != parsed.cols - 1) {
                fprintf(fp,"%c",delim);
            }
        }
        fprintf(fp,"\n");
    }

    return 0;
}

#endif

