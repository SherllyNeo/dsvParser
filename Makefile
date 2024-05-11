CFLAGS = -g3 -O -Wall -W -pedantic 

dsvParserExample src/main.c:
	mkdir -p bin
	gcc $(CFLAGS) src/main.c  -o bin/dsvParserExample 
