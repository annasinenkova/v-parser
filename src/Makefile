CC = g++ -std=c++11
CFLAGS = -g
LFLAGS =

all: prog

prog: main.o parser.o scanner.o circuit.o
	$(CC) $(LFLAGS) main.o parser.o scanner.o circuit.o -o prog
	
main.o: main.cpp parser.h
	$(CC) $(CFLAGS) -c $< -o $@
	
%.o: %.cpp %.h
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm *.o
	rm prog
