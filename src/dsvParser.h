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

char*** allocateContents(size_t rows, size_t cols, size_t strings_len) {
    char*** p = (char***)malloc(sizeof(char**) * rows);
    if (!p) {
        fprintf(stderr, "PARSE_ERR: Unable to allocate memory for rows\n");
        return NULL; // Return an invalid Parsed struct
    }
    for (size_t x = 0; x < rows; x++) {
        p[x] = (char**)malloc(sizeof(char*) * cols); /* over estimate size for cols (and values) */
        if (!p[x]) {
            fprintf(stderr, "PARSE_ERR: Unable to allocate memory for cols\n");
            // Free previously allocated memory
            for (size_t i = 0; i < x; i++) {
                for (size_t j = 0; j < cols; j++) {
                    free(p[i][j]);
                }
                free(p[i]);
            }
            free(p);
            return NULL; 
        }
        for (size_t y = 0; y < cols; y++) {
            p[x][y] = (char*)malloc(sizeof(char) * strings_len); /* over estimate size for strings */
            if (!p[x][y]) {
                fprintf(stderr, "PARSE_ERR: Unable to allocate memory for strings\n");
                // Free previously allocated memory
                for (size_t i = 0; i <= x; i++) {
                    for (size_t j = 0; j < y; j++) {
                        free(p[i][j]);
                    }
                    free(p[i]);
                }
                free(p);
                return NULL;
            }
        }
    }
    return p;
}

char*** reallocContents(char*** p, size_t old_rows, size_t old_cols, size_t new_rows, size_t new_cols, size_t strings_len) {
    // Reallocate memory for rows
    char*** tmp = (char***)realloc(p, sizeof(char**) * new_rows);
    if (!tmp) {
        fprintf(stderr, "PARSE_ERR: Unable to reallocate memory for rows\n");
        return NULL;
    }
    p = tmp;

    // Reallocate memory for columns and strings
    for (size_t x = 0; x < old_rows; x++) {
        // Reallocate memory for columns
        char** tmpC = (char**)realloc(p[x], sizeof(char*) * new_cols);
        if (!tmpC) {
            fprintf(stderr, "PARSE_ERR: Unable to reallocate memory for cols\n");
            // Free previously allocated memory
            for (size_t i = 0; i < x; i++) {
                free(p[i]);
            }
            free(p);
            return NULL;
        }
        p[x] = tmpC;

        // Reallocate memory for strings
        for (size_t y = 0; y < old_cols; y++) {
            char* tmpS = (char*)realloc(p[x][y], sizeof(char) * strings_len);
            if (!tmpS) {
                fprintf(stderr, "PARSE_ERR: Unable to reallocate memory for strings\n");
                // Free previously allocated memory
                for (size_t i = 0; i <= x; i++) {
                    for (size_t j = 0; j < y; j++) {
                        free(p[i][j]);
                    }
                    free(p[i]);
                }
                free(p);
                return NULL;
            }
            p[x][y] = tmpS;
        }

        // Allocate memory for additional columns and strings (if needed)
        for (size_t y = old_cols; y < new_cols; y++) {
            p[x][y] = (char*)malloc(sizeof(char) * strings_len);
            if (!p[x][y]) {
                fprintf(stderr, "PARSE_ERR: Unable to allocate memory for strings\n");
                // Free previously allocated memory
                for (size_t i = 0; i <= x; i++) {
                    for (size_t j = 0; j < y; j++) {
                        free(p[i][j]);
                    }
                    free(p[i]);
                }
                free(p);
                return NULL;
            }
        }
    }

    // Allocate memory for additional rows, columns, and strings (if needed)
    for (size_t x = old_rows; x < new_rows; x++) {
        p[x] = (char**)malloc(sizeof(char*) * new_cols);
        if (!p[x]) {
            fprintf(stderr, "PARSE_ERR: Unable to allocate memory for cols\n");
            // Free previously allocated memory
            for (size_t i = 0; i < x; i++) {
                free(p[i]);
            }
            free(p);
            return NULL;
        }

        for (size_t y = 0; y < new_cols; y++) {
            p[x][y] = (char*)malloc(sizeof(char) * strings_len);
            if (!p[x][y]) {
                fprintf(stderr, "PARSE_ERR: Unable to allocate memory for strings\n");
                // Free previously allocated memory
                for (size_t i = 0; i <= x; i++) {
                    for (size_t j = 0; j < y; j++) {
                        free(p[i][j]);
                    }
                    free(p[i]);
                }
                free(p);
                return NULL;
            }
        }
    }

    return p;
}


Parsed parse_source(char* source, size_t size, char delim) {

    size_t cur = 0;

    size_t rows_est = 1;
    size_t cols_est = 1;
    size_t strings_len_est = 1;
    Parsed returnVal = { NULL, 0, 0, false };

    returnVal.content = allocateContents(rows_est, cols_est, strings_len_est);
    if (returnVal.content == NULL) {
        /* failed to allocate */
    }




    bool valid = false;
    bool escaping = false;
    size_t x = 0;
    size_t y = 0;

    size_t cols = 0;

    for (size_t i = 0; i < size; i++) {
        printf("x: %zu, y: %zu, cur: %zu | rows_est: %zu, cols_est: %zu, strings_len_est: %zu\n",x,y,cur,rows_est,cols_est,strings_len_est);
        /* if any of the variables exceed their estimate, reallocate */
        if (x >= rows_est || y >= cols_est || cur >= strings_len_est) {
            size_t new_rows_est = rows_est;
            size_t new_cols_est = cols_est;
            size_t new_strings_len_est = strings_len_est;

            // Update estimation variables if exceeded
            if (x >= rows_est) new_rows_est *= 2;
            if (y >= cols_est) new_cols_est *= 2;
            if (cur >= strings_len_est) new_strings_len_est *= 2;

            // Reallocate contents
            char*** tmp = reallocContents(returnVal.content, rows_est, cols_est, new_rows_est, new_cols_est, new_strings_len_est);
            if (!tmp) {
                // Reallocation failed, handle error
                fprintf(stderr, "PARSE_ERR: Unable to reallocate memory for contents\n");
                return returnVal;
            }

            // Update variables with new estimates
            returnVal.content = tmp;
            rows_est = new_rows_est;
            cols_est = new_cols_est;
            strings_len_est = new_strings_len_est;
        }
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

