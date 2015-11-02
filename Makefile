CC = gcc
GUILE_CFLAGS = $(shell pkg-config guile-2.0 --cflags)
CFLAGS = -std=c99 -pedantic -Wall $(GUILE_CFLAGS) -g
SOURCES = journal.c
SNARFS = $(SOURCES:.c=.x)
BIN = libguile-journal.so

LIBS = $(shell pkg-config guile-2.0 --libs) -lsystemd

SITEDIR = $(shell guile -c "(display (%site-dir))(newline)")
SITECCACHEDIR = $(shell guile -c "(display (%site-ccache-dir))(newline)")
EXTENSIONDIR = $(shell guile -c "(display (assoc-ref %guile-build-info 'extensiondir))(newline)")

$(BIN): $(SNARFS) $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) -shared -o $@ -fPIC $(SOURCES)

%.x: %.c
	guile-snarf -o $@ $< $(GUILE_CFLAGS)

journal.go: journal.scm
	guild compile $< -o $@

install: journal.go
	install -p -m 755 $(BIN) -D $(EXTENSIONDIR)/$(BIN)
	install -p -m 644 journal.scm -D $(SITEDIR)/systemd/journal.scm
	install -p -m 644 journal.go -D $(SITECCACHEDIR)/journal.go

uninstall:
	rm -f $(EXTENSIONDIR)/$(BIN)
	rm -f $(SITEDIR)/systemd
	rm -f $(SITECCACHEDIR)/journal.go

clean:
	rm -f journal.x libguile-journal.so journal.go

.PHONY: clean install uninstall
