all: bench

bench: bench.c
	$(CC) bench.c -o bench -O2 -Wall -W -std=c99

clean:
	rm -f bench