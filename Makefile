CC=gcc
CFLAGS=-I.
DEPS =
OBJ = uniqueidserver.o
EXE = uniqueidserver

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE): $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

clean:
	rm *.o $(EXE)
