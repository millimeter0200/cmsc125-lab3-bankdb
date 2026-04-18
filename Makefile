CC = gcc

CFLAGS = -Wall -Wextra -std=c99 -O2 -pthread
DEBUG_FLAGS = -Wall -Wextra -std=c99 -g -fsanitize=thread -pthread

SRC = src/main.c src/parser.c src/bank.c src/timer.c
OUT = bankdb

all:
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

debug:
	$(CC) $(DEBUG_FLAGS) $(SRC) -o $(OUT)

test: all
	./bankdb tests/accounts.txt tests/trace_simple.txt
	./bankdb tests/accounts.txt tests/trace_readers.txt
	./bankdb tests/accounts.txt tests/trace_deadlock.txt
	./bankdb tests/accounts.txt tests/trace_abort.txt
	./bankdb tests/accounts.txt tests/trace_buffer.txt

clean:
	rm -f $(OUT)