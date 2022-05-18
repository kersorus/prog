CFLAGS=-Wall -Wextra -pthread -lm

.PHONY: compile clean

compile: integral.c
	gcc -o integral $^ $(CFLAGS)

clean:
	rm -f *.o integral
