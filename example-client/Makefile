# I'm using clang but gcc should work too.
CC=clang++
# We must include the "include" directory which contains the header file for this library.
CFLAGS=-I../include
# We must link with the system jack shared library.
LDFLAGS=-ljack -g

run: main.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) main.cpp -o run
