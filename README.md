# dsvParser

## A simple CSV or other deliminated parser 

This is a header only library. Simpily include the headerfile and use!

Look at the example main.c to see all the functionality.

```bash
git clone https://github.com/SherllyNeo/dsvParser.git &&
cd dsvParser &&
make &&
./bin/dsvParserExample
```


### Features

```c
typedef struct {
    char*** content;
    size_t rows;
    size_t cols;
    bool valid;
} DSV;
```



* The struct can be checked to see if it is valid after each operation
* Read delim seperated value file into a DSV struct (
* Escapes inner deliminators with "", for example one,"tw,o",three will be parsed correctly
* Supports '\\' escaping, so \, will not start a newline in a csv for example
* Write DSV to a file with a customer deliminator which allows for escaping
* Loop through each element of the array using the rows and cols to do with what you like
* Insert row
* Delete row
* print DSV
* Easy to free using the dsvFreeDSV function using the dsvFreeDSV function
* Most functions returns an int value where 0 means success or a DSV struct that may be valid


### Functions

```c
DSV dsvParseFile(char* filepath, char delim);
```

```c
int dsvWriteFile(DSV parsed,char* filepath,char delim);
```


```c
int dsvInsertRow(DSV *dsv, char** tmp_row, size_t position);
```

```c
int dsvRemoveRow(DSV *dsv, size_t position);
```

```c
void dsvPrintDSV(DSV parsed);
```

```c
int dsvFreeDSV(DSV parsed);
```


you can also access elements directly like so (example printing)
```c
for (size_t i = 0; i<parsed_csv.rows; i++) {
    for (size_t j = 0; j<parsed_csv.cols; j++) {
        printf("%s ",parsed_csv.content[i][j]);
    }
    printf("\n");
}
```




