CC=clang++
CFLAGS=-Iinclude
LDFLAGS=-ljack -g

run: main.cpp
	$(CC) $(CFLAGS) $(LDFLAGS) main.cpp -o run
