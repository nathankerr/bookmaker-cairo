CFLAGS=`pkg-config --cflags cairo poppler-glib` -Wall -Werror -g
LDFLAGS=`pkg-config --libs cairo poppler-glib`

bookmaker: main.o options.o page.o pdf.o cropbox.o layout.o
	$(CC) -o $@ $+ $(LDFLAGS)

%.o: %.c all.h
	$(CC) -c $< $(CFLAGS)

clean:
	rm -rf bookmaker bookmaker.dSYM *.book*.pdf *.o
