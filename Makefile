CFLAGS=`pkg-config --cflags cairo poppler-glib` -Wall -Werror -g
LDFLAGS=`pkg-config --libs cairo poppler-glib` -L`brew --prefix gettext`/lib

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	PDFREADER := okular
endif
ifeq ($(UNAME), Darwin)
	PDFREADER := open
	PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/:/opt/X11/lib/pkgconfig/
	LDFLAGS+=-L`brew --prefix gettext`/lib
endif

book: bookmaker
	rm -f book.pdf
	./bookmaker PDF32000_2008-short.pdf book.pdf
	open book.pdf

bookmaker: main.o options.o page.o pdf.o cropbox.o layout.o
	$(CC) -o $@ $+ $(LDFLAGS)

%.o: %.c all.h
	$(CC) -c $< $(CFLAGS)

clean:
	rm -rf bookmaker bookmaker.dSYM *.book*.pdf *.o
