all: piboard piplay

CFLAGS	= `pkg-config --cflags gtk+-3.0 libmypaint nanomsg` -g

piboard: piboard.o mypaint-resizable-tiled-surface.o
	$(CC) -o $@ $^ `pkg-config --libs gtk+-3.0 libmypaint nanomsg` -lm

piplay: piplay.o
	$(CC) -o $@ $^ `pkg-config --libs nanomsg`

clean:
	rm -f piboard.o mypaint-resizable-tiled-surface.o piboard piplay

install: piboard
	install piboard /usr/bin
	xdg-desktop-menu install pi-classroom-piboard.desktop
	xdg-icon-resource install --size 24 pi-classroom-piboard-24.png p-classroom-2iboard
	xdg-icon-resource install --size 48 pi-classroom-piboard-48.png pi-classroom-piboard
	xdg-icon-resource install --size 64 pi-classroom-piboard-64.png pi-classroom-piboard
	xdg-desktop-menu forceupdate
