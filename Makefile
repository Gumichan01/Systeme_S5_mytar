
# $@ : nom de la cible
# $^ : liste des dependances 

CC=gcc
CFLAGS=-Wall -pedantic -g
LFLAGS=
EXEC=mytar

# Edition des liens + création de l'executable mytar
all: $(EXEC)



$(EXEC) : lock_lib.o param.o mytar.o main.o
	$(CC) -o $(EXEC) $^ $(LFLAGS)


lock_lib.o : lock_lib.c
	$(CC) -o $@ -c $^ $(CFLAGS)


param.o : param.c
	$(CC) -o $@ -c $^ $(CFLAGS)


mytar.o : mytar.c
	$(CC) -o $@ -c $^ $(CFLAGS) 


main.o : main.c
	$(CC) -o $@ -c $^ $(CFLAGS)


cleanall:

	rm -rf *.o $(EXEC)


