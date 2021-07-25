# piboard

##程序编译：
   程序编译需要预先安装如下的开发库(在Debian系统下)：
      libgtk-3.0-dev
      libnanomsg-dev
      libmypaint-dev
   编译命令为：
      gcc -o piboard `pkg-config --cflags gtk+-3.0 gdk-3.0 libmypaint nanomsg` piboard.c
##程序运行：
    用户端需要在/etc/piboard.conf文件中输入绘制端的IP地址/主机名，否则程序将作为绘制端运行。
    程序支持所有MyPaint画笔，如果存在，程序会采用/usr/share/mypaint-data/1.0/brushes/deevad/pen-note.myb画笔。
    所以在运行前，建议安装mypaint-brushes。
##程序特点：
    由于采用了PubSub模式，所以可以有多个查看端。
    由于采用了nanomsg，所以绘制端和查勘端运行顺序不分先后。
    由于采用了libmypaint，所以完全支持MyPaint的Brush，建议安装mypaint-brush。
    程序锁采用的Brush为pen-note.myb
