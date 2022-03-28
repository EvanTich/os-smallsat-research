CC=gcc
CFLAGS=-Wall -g

main: main.c
	$(CC) $(CFLAGS) -o main main.c

run: run_sleep run_custom

run_sleep: main
	./main --test 1

run_custom: main
	./main --test 3

.PHONY: clean
clean:
	rm -f main
