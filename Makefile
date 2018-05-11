#
#: Imposter
#
CFLAGS=-Wall -std=gnu99 -g

.PHONY: all
all: imposter

imposter: imposter.o
	gcc $(CFLAGS) -o $@ $^

imposter.o: imposter.c
	gcc $(CFLAGS) -c $<

.PHONY: clean
clean:
	@-find . -name "*~" -delete
	@-find . -name "*.o" -delete
	@-find . -name "imposter" -delete
