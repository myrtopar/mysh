mysh: mysh.c functions.c
	gcc -c mysh.c
	gcc -c functions.c
	gcc -c wildcard.c
	gcc -o mysh mysh.o functions.o wildcard.o

clean:
	rm -f mysh *.o