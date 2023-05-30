# thanks to the help from https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

CC=g++
CFLAGS=-I. -w
DEPS = main.h LCD.h Value.h Interpreter.h
OBJ = main.o LCD.o Value.o Interpreter.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

cc1284: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean avr

avr:
	avr-gcc -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o cc1284 