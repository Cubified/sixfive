all: sixfive

CC=gcc

LIBS=
CFLAGS=-Os -pipe -s -ansi -pedantic
DEBUGCFLAGS=-Og -pipe -g -ansi -pedantic -Wall -DDEBUG_BUILD

INPUT=sixfive.c
OUTPUT=sixfive

RM=/bin/rm

.PHONY: sixfive
sixfive:
	$(CC) $(INPUT) -o $(OUTPUT) $(LIBS) $(CFLAGS)

debug:
	$(CC) $(INPUT) -o $(OUTPUT) $(LIBS) $(DEBUGCFLAGS)

test:
	./$(OUTPUT)

clean:
	if [ -e $(OUTPUT) ]; then $(RM) $(OUTPUT); fi
