all: piboard

LIBS	= `pkg-config --libs gtk+-3.0 libmypaint nanomsg`
CFLAGS	= `pkg-config --cflags gtk+-3.0 libmypaint nanomsg` -g

piboard: piboard.o mypaint-resizable-tiled-surface.o
	$(CC) $(LIBS) -o $@ $^

clean:
	rm -f piboard.o piboard

install: piboard
	install piboard /usr/bin
	install piboard.png /usr/share/pixmaps
	install piboard.desktop /usr/share/applications
	lxpanelctl restart
	
