PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/:/opt/X11/lib/pkgconfig/
CFLAGS=`pkg-config --cflags cairo poppler-glib` -Wall -Werror -g
LDFLAGS=`pkg-config --libs cairo poppler-glib` -L`brew --prefix gettext`/lib

book: bookmaker
	rm -f book.pdf
	./bookmaker PDF32000_2008-short.pdf
	open book.pdf

clean:
	rm -rf bookmaker bookmaker.dSYM book.pdf even.pdf odd.pdf
