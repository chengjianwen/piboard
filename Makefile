all: piboard piplay

CFLAGS	= `pkg-config --cflags gtk+-3.0 libmypaint nanomsg` -g

piboard: piboard.o mypaint-resizable-tiled-surface.o
	$(CC) `pkg-config --libs gtk+-3.0 libmypaint nanomsg` -lm -o $@ $^

piplay: piplay.o
	$(CC) `pkg-config --libs nanomsg` -o $@ $^

clean:
	rm -f piboard.o mypaint-resizable-tiled-surface.o piboard piplay

install: piboard
	install piboard /usr/bin
	install piboard.png /usr/share/pixmaps
	install piboard.desktop /usr/share/applications
	lxpanelctl restart
