mysh: mysh.c functions.c history.c wildcard.c alias.c
	gcc -c mysh.c
	gcc -c functions.c
	gcc -c wildcard.c
	gcc -c history.c
	gcc -c alias.c
	gcc -o mysh mysh.o functions.o wildcard.o history.o alias.o

clean:
	rm -f mysh *.o
