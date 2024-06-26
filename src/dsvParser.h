#ifndef PARSER

#define PARSER

#define ERR_FILE 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* read_source(char* filepath,size_t* size) {
    /* SLURP the whole file and return the string and the size */

    FILE* fp = fopen(filepath,"r");
    if (!fp) {
        fprintf(stderr,"DSV_FILE_ERR: unable to open file %s\n",filepath);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size_t end = ftell(fp);
    rewind(fp);

    char* source = (char*)malloc(end+1);

    if (!source) {
        fprintf(stderr, "DSV_ALLOC_ERR: Failed to allocate memory for file content %s\n",filepath);
        fclose(fp);
        return NULL;
    }

    if (end <= 0) {
        fprintf(stderr, "DSV_FILE_ERR: Failed to find lenth of file content %s\n",filepath);
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
} DSV;


char*** allocateContents(size_t rows, size_t cols, size_t strings_len) {
    /* allocate the 3D char array with some error handling */
    char*** p = (char***)malloc(sizeof(char**) * rows);
    if (!p) {
        fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for rows\n");
        return NULL; 
    }
    for (size_t x = 0; x < rows; x++) {
        p[x] = (char**)malloc(sizeof(char*) * cols); 
        if (!p[x]) {
            fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for cols\n");
            /* free previously allocated memory, this is a pain */
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
            p[x][y] = (char*)malloc(sizeof(char) * strings_len); 
            if (!p[x][y]) {
                fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for strings\n");
                /* free previously allocated memory, this is a pain */
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

size_t max_string_length(char** strings) {
    /* helper function to find the max size to allocate in an array */
    size_t max_length = 0;
    size_t max_iter = 100;

    for (size_t i = 0; strings[i] != NULL; i++) {
        if (i >= max_iter) {
            fprintf(stderr, "DSV_USER_ERR: Ran for a %zu iterations without finding null termination in max_string_length... exiting\n",max_iter);
            break;
        }
        size_t length = strlen(strings[i]);
        if (length > max_length) {
            max_length = length;
        }
    }

    return max_length;
}

char*** reallocContents(char*** p, size_t old_rows, size_t old_cols, size_t new_rows, size_t new_cols, size_t strings_len) {
    /* helper function to resize 3D char array */
    char*** tmp = (char***)realloc(p, sizeof(char**) * new_rows);
    if (!tmp) {
        fprintf(stderr, "DSV_ALLOC_ERR: Unable to reallocate memory for rows\n");
        return NULL;
    }
    p = tmp;

    /* resize old columns and rows */
    for (size_t x = 0; x < old_rows; x++) {
        char** tmpC = (char**)realloc(p[x], sizeof(char*) * new_cols);
        if (!tmpC) {
            fprintf(stderr, "DSV_ALLOC_ERR: Unable to reallocate memory for cols\n");
            for (size_t i = 0; i < x; i++) {
                free(p[i]);
            }
            free(p);
            return NULL;
        }
        p[x] = tmpC;

        for (size_t y = 0; y < old_cols; y++) {
            char* tmpS = (char*)realloc(p[x][y], sizeof(char) * strings_len);
            if (!tmpS) {
                fprintf(stderr, "DSV_ALLOC_ERR: Unable to reallocate memory for strings\n");
                /* free old memory */
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

        /* add new columns and rows and strings with correct dimensions */
        for (size_t y = old_cols; y < new_cols; y++) {
            p[x][y] = (char*)malloc(sizeof(char) * strings_len);
            if (!p[x][y]) {
                fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for strings\n");
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

    for (size_t x = old_rows; x < new_rows; x++) {
        p[x] = (char**)malloc(sizeof(char*) * new_cols);
        if (!p[x]) {
            fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for cols\n");
            for (size_t i = 0; i < x; i++) {
                free(p[i]);
            }
            free(p);
            return NULL;
        }

        for (size_t y = 0; y < new_cols; y++) {
            p[x][y] = (char*)malloc(sizeof(char) * strings_len);
            if (!p[x][y]) {
                fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for strings\n");
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


DSV parse_source(char* source, size_t size, char delim) {
    /* the most useful function, reads in a CSV and parses it */

    /* assumptions of typical csv sizes */
    size_t rows_est = 5000;
    size_t cols_est = 100;
    size_t strings_len_est = 1000;
    DSV returnVal = { NULL, 0, 0, false };

    returnVal.content = allocateContents(rows_est, cols_est, strings_len_est);
    if (returnVal.content == NULL) {
        /* failed to allocate */
        fprintf(stderr,"DSV_ALLOC_ERR: Failed to allocate memory for parsed content\n");
        return returnVal;
    }




    /* init values */
    size_t cur = 0;
    bool valid = false;
    bool escaping = false;
    size_t x = 0;
    size_t y = 0;
    size_t cols = 0;

    for (size_t i = 0; i < size; i++) {
        /* if any of the variables exceed their estimate, reallocate */
        if (x >= rows_est || y >= cols_est || cur >= strings_len_est) {
            size_t new_rows_est = rows_est;
            size_t new_cols_est = cols_est;
            size_t new_strings_len_est = strings_len_est;

            /* Update estimation variables if required */
            if (x >= rows_est) new_rows_est *= 2;
            if (y >= cols_est) new_cols_est *= 2;
            if (cur >= strings_len_est) new_strings_len_est *= 2;

            /* big reallocate for contents */
            char*** tmp = reallocContents(returnVal.content, rows_est, cols_est, new_rows_est, new_cols_est, new_strings_len_est);
            if (!tmp) {
                // Reallocation failed, handle error
                fprintf(stderr, "DSV_ALLOC_ERR: Unable to reallocate memory for contents\n");
                return returnVal;
            }

            /* update all variables */
            returnVal.content = tmp;
            rows_est = new_rows_est;
            cols_est = new_cols_est;
            strings_len_est = new_strings_len_est;
        }

        /* Grab current character */
        char current_char = source[i];

        /* check if we need to add the next character literally, if it the next character exists " */
        if (current_char == '\\' && (i < (size - 1))) {
            returnVal.content[x][y][cur++] = source[++i];
        }
        /* toggle escaping if we see " */
        else if (current_char == '"') {
            escaping = !escaping;
        }
        /* only parse below if we are not escaping */
        else if (current_char == delim && !escaping) {
            /* null terminate the current column, reset cursor */
            returnVal.content[x][y][cur] = '\0';
            y++;
            cur = 0;

        } else if (current_char == '\n' && !escaping) {
            /* null terminate the current row, reset cursor and column count, but store value in cols*/
            returnVal.content[x][y][cur] = '\0';
            x++;
            cols = (size_t)y;
            y = 0;
            cur = 0;
        } else {
            /* add to cell */
            returnVal.content[x][y][cur++] = current_char;
        }
    }

    /* if all worked, we want to set the correct values and return */
    valid = true;
    returnVal.valid = valid;
    returnVal.rows = x;
    returnVal.cols = cols + 1;

    free(source);

    return returnVal;
}


/* PUBLIC FUNCTIONS */

int dsvInsertRow(DSV *dsv, char** tmp_row, size_t position) {

    size_t max_size_of_string_in_tmp_row = max_string_length(tmp_row);
    size_t length = 0;
    int max_iter = 100000;
    for (int n = 0; tmp_row[n] != NULL; n++) {
        if (n > max_iter) {
            fprintf(stderr, "DSV_ALLOC_ERR: Unable to measure tmp row past [%d]\n", n);
        }
        length++;
    }

    /* insure position is in dsv, position will always be positive */
    if (position > dsv->rows) {
        fprintf(stderr,"DSV_USR_ERR: position to try and insert row (%zu) is out of bounds (MAX: %zu)\n",position,dsv->rows);
        return 1;
    }

    /* truncate char** row to match the required number of columns and NULL terminate */
    char** row = (char**)malloc(sizeof(char*) * (dsv->cols+1));
    if (!row) {
        fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for row\n");
        return 1;
    }

    for (size_t i = 0; i<dsv->cols; i++) {
        row[i] = (char*)malloc(sizeof(char*)*max_size_of_string_in_tmp_row+1);
        if (!row[i]) {
            fprintf(stderr, "DSV_ALLOC_ERR: Unable to allocate memory for row[%zu]\n", i);
            /* free old mem */
            for (size_t j = 0; j < i; j++) {
                free(row[j]);
            }
            free(row);
            return 1;
        }
        strncpy(row[i], tmp_row[i],max_size_of_string_in_tmp_row); 
        row[i][max_size_of_string_in_tmp_row] = '\0';
    }

    /* free all memory from tmp row */
    for (int n = (int)dsv->cols; n < (int)length; n++) {
        if (n > max_iter) {
            fprintf(stderr, "DSV_ALLOC_ERR: Unable to free memory for value [%d]\n", n);
        }
        if (tmp_row[n]) {
            free(tmp_row[n]);
        }
    }
    free(tmp_row);

    /* null terminate new */
    row[dsv->cols] = NULL;

    /* Find the largest string out of all rows */
    size_t max_size_of_string_in_new_row = max_string_length(row);
    size_t max_size_of_string_in_dsv = max_size_of_string_in_new_row;
    for (size_t j = 0; j<dsv->rows; j++) {
        size_t max_of_cur_row = max_string_length(dsv->content[j]);
        if (max_of_cur_row > max_size_of_string_in_dsv) {
            max_size_of_string_in_dsv = max_of_cur_row;
        }
    }

    char*** tmp = reallocContents(dsv->content, dsv->rows, dsv->cols, dsv->rows+1, dsv->cols,max_size_of_string_in_dsv);
    if (!tmp) {
        fprintf(stderr, "DSV_ALLOC_ERR: Unable to reallocate memory for rows\n");
        dsv->valid = false;
        return 1;
    }
    dsv->content = tmp;
    dsv->rows++;

    /* shift all rows */
    for (size_t i = dsv->rows - 1; i > position; i--) {
        dsv->content[i] = dsv->content[i - 1];
    }

    /* insert new */
    dsv->content[position] = row;

    return 0;
}

int dsvRemoveRow(DSV *dsv, size_t position) {

    /* insure position is in dsv, position will always be positive */
    if (position > dsv->rows) {
        fprintf(stderr,"DSV_USR_ERR: position to try and delete row (%zu) is out of bounds (MAX: %zu)\n",position,dsv->rows);
        return 1;
    }

     /* Free memory for the row being removed */
    for (size_t i = 0; i < dsv->cols; i++) {
        free(dsv->content[position][i]);
    }
    free(dsv->content[position]);

    /* Shift subsequent rows up */
    for (size_t i = position; i < dsv->rows - 1; i++) {
        dsv->content[i] = dsv->content[i + 1];
    }

    /* Decrement the row count */
    dsv->rows--;

    return 0;
}


void dsvPrintDSV(DSV parsed) {
    /* helper function to print parsed */
    printf("rows: %zu, cols: %zu\n",parsed.rows,parsed.cols);
    for (size_t i = 0; i < parsed.rows; i++) {
        for (size_t j = 0; j < parsed.cols; j++) {
            printf("|%s| ",parsed.content[i][j]);
        }
        printf("\n");
    }
}

int dsvFreeDSV(DSV parsed) {
    /* always free */
    for (size_t i = 0; i < parsed.rows; i++) {
        for (size_t j = 0; j < parsed.cols; j++) {
            if (parsed.content[i][j]) {
                free(parsed.content[i][j]);
            }
        }
        if (parsed.content[i]) {
            free(parsed.content[i]);
        }
    }
    if (parsed.content) {
        free(parsed.content);
    }
    return 0;
}


DSV dsvParseFile(char* filepath, char delim) {
    /* Parse a file into the DSV object */
    DSV returnVal = { NULL, 0, 0, false };
    if (delim == '"' || delim == '\n') {
        fprintf(stderr,"DELIM cannot be \" or \\n \n");
        return returnVal;
    }

    size_t size = 0;

    char* source = read_source(filepath,&size);
    if (!source) {
        fprintf(stderr, "DSV_FILE_ERR: failed to read %s\n",filepath);
        exit(ERR_FILE);
    }

    /* this function frees source */
    returnVal = parse_source(source,size,delim);
    if (!returnVal.valid || returnVal.rows == 0 || returnVal.cols == 0) {
        fprintf(stderr, "DSV_FILE_ERR: failed to read %s\n",filepath);
        return returnVal;
    }

    return returnVal;
}

int dsvWriteFile(DSV parsed,char* filepath,char delim) {
    /* write DSV object to file */
    FILE* fp = fopen(filepath,"w");
    if (!fp) {
        fprintf(stderr,"DSV_FILE_ERR: unable to open file to write to %s\n",filepath);
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

    fclose(fp);

    return 0;
}

#endif

