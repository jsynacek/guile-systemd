CC = gcc
GUILE_CFLAGS = $(shell pkg-config guile-2.0 --cflags)
CFLAGS = -std=c99 -pedantic -Wall -Werror=implicit-function-declaration $(GUILE_CFLAGS) -g
LIBS = $(shell pkg-config guile-2.0 --libs) -lsystemd

SITEDIR = $(shell guile -c "(display (%site-dir))(newline)")
SITECCACHEDIR = $(shell guile -c "(display (%site-ccache-dir))(newline)")
EXTENSIONDIR = $(shell guile -c "(display (assoc-ref %guile-build-info 'extensiondir))(newline)")

all: libguile-journal.so libguile-daemon.so

libguile-journal.so: common.o journal.c journal.x
	$(CC) $(CFLAGS) $(LIBS) common.o -shared -o $@ -fPIC journal.c

libguile-daemon.so: common.o daemon.c daemon.x
	$(CC) $(CFLAGS) $(LIBS) common.o -shared -o $@ -fPIC daemon.c

common.o: common.c common.h
	$(CC) $(CFLAGS) $(GUILE_CFLAGS) -fPIC -c $<

%.x: %.c
	guile-snarf -o $@ $< $(GUILE_CFLAGS)

%.go: %.scm
	guild compile $< -o $@

install: journal.go daemon.go
	install -p -m 755 libguile-journal.so -D $(EXTENSIONDIR)/libguile-journal.so
	install -p -m 755 libguile-daemon.so -D $(EXTENSIONDIR)/libguile-daemon.so
	install -p -m 644 journal.scm -D $(SITEDIR)/systemd/journal.scm
	install -p -m 644 daemon.scm -D $(SITEDIR)/systemd/daemon.scm
	install -p -m 644 journal.go -D $(SITECCACHEDIR)/journal.go
	install -p -m 644 daemon.go -D $(SITECCACHEDIR)/daemon.go

uninstall:
	rm -f $(EXTENSIONDIR)/libguile-journal.so
	rm -f $(EXTENSIONDIR)/libguile-daemon.so
	rm -f $(SITEDIR)/systemd
	rm -f $(SITECCACHEDIR)/journal.go
	rm -f $(SITECCACHEDIR)/daemon.go

clean:
	rm -f journal.x daemon.x common.o libguile-journal.so libguile-daemon.so journal.go daemon.go

.PHONY: clean install uninstall
