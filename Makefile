.PHONY:

CONFIGFILE = config.mk
include $(CONFIGFILE)

all: median

median: median.o
	$(CC) -o $@ median.o $(LDFLAGS)

median.o: median.c
	$(CC) -c -o $@ median.c $(CFLAGS) $(CPPFLAGS)

check: median
	test "$$(printf '%s\n' 1 5 2 | ./median)" = 2
	test "$$(printf '%s\n' 1 2 | ./median)" = 1.5
	test "$$(printf '%s\n' '1 a' '2 a' | ./median)" = '1.5 a'
	test "$$(printf '%s\n' '1 a' '2 b' | ./median)" = "$$(printf '%s\n' '1 a' '2 b')"
	test "$$(printf '%s\n' '1 a' '2 b' '3 a' | ./median)" = "$$(printf '%s\n' '2 a' '2 b')"
	test "$$(printf '%s\n' '1 a' '2 b' '3 a' '10 a' | ./median)" = "$$(printf '%s\n' '3 a' '2 b')"
	test "$$(printf '%s\n' '1' '2 ' | ./median)" = "$$(printf '%s\n' '1' '2 ')"
	test "$$(printf '1\ta\n3 a\n' | ./median)" = "$$(printf '1\ta\n3 a\n')"
	test "$$(printf '%s\n' a c d | ./median)" = c
	test "$$(printf '%s\n' a d c | ./median)" = c
	test "$$(printf '%s\n' a b | ./median)" = a
	test "$$(printf '%s\n' a b b | ./median)" = b

install:
	mkdir -p -- "$(DESTDIR)$(PREFIX)/bin"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man1"
	cp -- median "$(DESTDIR)$(PREFIX)/bin/"
	cp -- median.1 "$(DESTDIR)$(MANPREFIX)/man1/"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/bin/median"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man1/median.1"

clean:
	-rm -f -- median *.o

.SUFFIXES:
.SUFFIXES: .c .o

.PHONY: all check install uninstall clean
