# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Iinclude -pthread

# Targets
all: bin/server bin/client

bin/server: src/server/*.c
	$(CC) $(CFLAGS) $^ -o $@

bin/client: src/client/*.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f bin/server bin/client data/users.txt 