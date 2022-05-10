CFLAGS=--coverage -Wall -Wextra -MMD

.PHONY: clean run valgrind coverage

run: test
	./test

test: d_spisok.o test.o
	gcc -o test --coverage $^

valgrind: test
	valgrind ./test

coverage: run
	gcov d_spisok

clean:
	rm -f *.o *.d *.gcno *.gcda *.gcov test
	
include $(wildcard *.d)
