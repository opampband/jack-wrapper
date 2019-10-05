CC=clang++
LDFLAGS=-ljack -g

run: main.cpp
	$(CC) $(LDFLAGS) main.cpp -o run
