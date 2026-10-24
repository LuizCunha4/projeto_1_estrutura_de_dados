CC = cc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
CPPFLAGS = -Ilib
LDLIBS = -lm

SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard lib/*.h) src/reader_privado.h

.PHONY: all clean

all: trabalho

trabalho: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SOURCES) $(LDLIBS) -o trabalho

clean:
	rm -f trabalho
