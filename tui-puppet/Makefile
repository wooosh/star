PREFIX = /usr/local

CFLAGS += `pkg-config --cflags vterm libcrypto`
LDFLAGS += `pkg-config --libs vterm libcrypto`
LDFLAGS += -lutil

.PHONY: all clean install uninstall

all: tui-puppet

tui-puppet: tuipuppet.c keymap.h
	$(CC) $(CFLAGS) -o tui-puppet tuipuppet.c  $(LDFLAGS)

clean:
	rm tui-puppet

install: tui-puppet
	install -m 755 tui-puppet $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/tui-puppet
