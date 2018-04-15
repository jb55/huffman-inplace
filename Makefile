
CFLAGS=-Ideps -std=c99 -g -O0 -Wall

DEPS=test.o deps/hash/hash.o

huffi: $(DEPS)
	$(CC) -lm $^ -o $@

clean:
	rm -f $(DEPS)
