CC:= gcc
CFLAGS:=-Wall -Wextra -std=c99 -MMD

.PHONY: all clean

all: server client

server: server.o 
	$(CC) -o $@ $^

client: client.o integral.o
	$(CC) -o $@ $^ -lm -pthread

-include $(wildcard *.d)

clean:
	rm -f server client *.o *.d
