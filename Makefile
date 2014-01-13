CC=gcc
CFLAGS=-I.
DEPS =
OBJ = uniqidserver.o
EXE = uniqidserver

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm *.o $(EXE)
