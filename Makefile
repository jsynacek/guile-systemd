CC = gcc
GUILE_CFLAGS = $(shell pkg-config guile-2.0 --cflags)
CFLAGS = -std=c99 -pedantic -Wall $(GUILE_CFLAGS) -g

LIBS = $(shell pkg-config guile-2.0 --libs) -lsystemd

all: libguile-journal.so

libguile-journal.so: journal.c
	guile-snarf -o journal.x $< $(GUILE_CFLAGS)
	$(CC) $(CFLAGS) $(LIBS) -shared -o libguile-journal.so -fPIC journal.c

