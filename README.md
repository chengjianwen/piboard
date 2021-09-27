# piboard

PiBoard是一个支持远程绘制的白板软件，分为绘制端和用户端，一个绘制端同时可以支持多个用户端。

作为用户端运行时需要在/etc/piboard.conf文件中输入绘制端的IP地址/主机名，否则程序将作为绘制端运行。

通过piplay可以重新运行绘制端的绘制操作。

##程序编译：

   程序编译需要预先安装如下的开发库(在Debian系统下)：

      libgtk-3-dev

      libnanomsg-dev

      libmypaint-dev

   编译命令为：

      gcc -o piboard `pkg-config --cflags gtk+-3.0 libmypaint nanomsg` -lm piboard.c
      gcc -o piplay `pkg-config --cflags nanomsg` piplay.c

##程序运行：

    程序支持所有MyPaint画笔，画笔文件保存在./brushes目录下。

##程序特点：

    由于采用了PubSub模式，所以可以有多个用户端。

    由于采用了nanomsg，所以绘制端和用户端运行顺序不分先后。

    由于采用了libmypaint，所以完全支持MyPaint的Brush。

    程序所采用的Brush为deevad/liner.myb.
