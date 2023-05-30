CC=g++
CFLAGS=-I. -w
SRC = test.cpp ../LCD.cpp ../Interpreter.cpp ../Value.cpp


.PHONY: clean test

test: $(SRC)
	g++ -I../ test.cpp ../LCD.cpp ../Interpreter.cpp ../Value.cpp -o test $(CFLAGS)


clean:
	rm -f *.o test
