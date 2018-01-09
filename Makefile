CC=gcc
CFLAGS=`pkg-config --cflags glib-2.0` `pkg-config --cflags gtk+-2.0`   -g
LDFLAGS=`pkg-config --libs  glib-2.0` `pkg-config --libs gtk+-2.0`  -lzgui -g -lX11
APP=zmenu
PREFIX?=/usr

all:$(APP)
$(APP):zmenu.c
	gcc $(CFLAGS) $< $(LDFLAGS) -o $(APP)
	gcc $(CFLAGS) menu.c $(LDFLAGS) -o menu
clean:
	rm $(APP)
install:
	sudo install zmenu $(PREFIX)/bin/
