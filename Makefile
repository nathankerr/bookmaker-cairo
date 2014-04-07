PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/:/opt/X11/lib/pkgconfig/
CFLAGS=`pkg-config --cflags cairo poppler-glib` -Wall -Werror -g
LDFLAGS=`pkg-config --libs cairo poppler-glib` -L`brew --prefix gettext`/lib

book.pdf: bookmaker test.pdf
	rm -f book.pdf
	./bookmaker test.pdf
	open book.pdf

.PHONY: valgrind

valgrind: bookmaker
	valgrind --suppressions=local.supp --gen-suppressions=yes --leak-check=yes --leak-check=full --show-leak-kinds=all -v ./bookmaker test.pdf

clean:
	rm -f bookmaker book.pdf
