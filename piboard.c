/*
 *   程序名：piboard.c
 *     作者：成建文 <chengjianwen@gmail.com>
 *   修改日志: 
 *            2021-12-16	内嵌画笔kabura.myb到源代码
 */

#include <gtk/gtk.h>
#include <mypaint-brush.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include "mypaint-resizable-tiled-surface.h"

#pragma pack(4)

// 缺省压感值
#define	DEFAULT_PRESSURE	0.5
// 缺省X倾角值
#define	DEFAULT_XTILT		0.0
// 缺省Y倾角值
#define	DEFAULT_YTILT		0.0
/*
/usr/share/mypaint-data/1.0/brushes/classic/kabura.myb
*/
static char *brush = "{"
                     "\"comment\": \"MyPaint brush file\", "
                     "\"group\": \"\", "
                     "\"parent_brush_name\": \"classic/kabura\", "
                     "\"settings\": {"
                     "    \"anti_aliasing\": {"
                     "        \"base_value\": 0.93, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"change_color_h\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"change_color_hsl_s\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"change_color_hsv_s\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"change_color_l\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"change_color_v\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"color_h\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"color_s\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"color_v\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"colorize\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"custom_input\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"custom_input_slowness\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"dabs_per_actual_radius\": {"
                     "        \"base_value\": 2.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"dabs_per_basic_radius\": {"
                     "        \"base_value\": 3.24, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"dabs_per_second\": {"
                     "        \"base_value\": 48.87, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"direction_filter\": {"
                     "        \"base_value\": 2.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"elliptical_dab_angle\": {"
                     "        \"base_value\": 90.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"elliptical_dab_ratio\": {"
                     "        \"base_value\": 1.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"eraser\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"hardness\": {"
                     "        \"base_value\": 0.43, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"lock_alpha\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"offset_by_random\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"offset_by_speed\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"offset_by_speed_slowness\": {"
                     "        \"base_value\": 1.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"opaque\": {"
                     "        \"base_value\": 1.29, "
                     "        \"inputs\": {"
                     "            \"pressure\": ["
                     "                ["
                     "                  0.0, "
                     "                  -0.989583"
                     "                ], "
                     "                ["
                     "                  0.38253, "
                     "                  -0.59375"
                     "                ], "
                     "                ["
                     "                  0.656627, "
                     "                  0.041667"
                     "                ], "
                     "                ["
                     "                  1.0, "
                     "                  1.0"
                     "                ]"
                     "            ]"
                     "        }"
                     "    }, "
                     "    \"opaque_linearize\": {"
                     "        \"base_value\": 0.29, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"opaque_multiply\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {"
                     "            \"pressure\": ["
                     "                ["
                     "                  0.0, "
                     "                  0.0"
                     "                ], "
                     "                ["
                     "                  0.015, "
                     "                  0.0"
                     "                ], "
                     "                ["
                     "                  0.069277, "
                     "                  0.9375"
                     "                ], "
                     "                ["
                     "                  0.25, "
                     "                  1.0"
                     "                ], "
                     "                ["
                     "                  1.0, "
                     "                  1.0"
                     "                ]"
                     "            ]"
                     "        }"
                     "    }, "
                     "    \"radius_by_random\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"radius_logarithmic\": {"
                     "        \"base_value\": 0.92, "
                     "        \"inputs\": {"
                     "            \"pressure\": ["
                     "                ["
                     "                  0.0, "
                     "                  -0.7875"
                     "                ], "
                     "                ["
                     "                  0.237952, "
                     "                  -0.6"
                     "                ], "
                     "                ["
                     "                  0.5, "
                     "                  -0.15"
                     "                ], "
                     "                ["
                     "                  0.76506, "
                     "                  0.6"
                     "                ], "
                     "                ["
                     "                  1.0, "
                     "                  0.9"
                     "                ]"
                     "            ]"
                     "        }"
                     "    }, "
                     "    \"restore_color\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"slow_tracking\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"slow_tracking_per_dab\": {"
                     "        \"base_value\": 10.0, "
                     "        \"inputs\": {"
                     "            \"speed1\": ["
                     "                ["
                     "                  0.0, "
                     "                  -1.428571"
                     "                ], "
                     "                ["
                     "                  4.0, "
                     "                  10.0"
                     "                ]"
                     "            ], "
                     "            \"speed2\": ["
                     "                ["
                     "                  0.0, "
                     "                  -1.428571"
                     "                ], "
                     "                ["
                     "                  4.0, "
                     "                  10.0"
                     "                ]"
                     "            ]"
                     "        }"
                     "    }, "
                     "    \"smudge\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"smudge_length\": {"
                     "        \"base_value\": 0.5, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"smudge_radius_log\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"speed1_gamma\": {"
                     "        \"base_value\": 2.87, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"speed1_slowness\": {"
                     "        \"base_value\": 0.04, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"speed2_gamma\": {"
                     "        \"base_value\": 4.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"speed2_slowness\": {"
                     "        \"base_value\": 0.8, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"stroke_duration_logarithmic\": {"
                     "        \"base_value\": 4.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"stroke_holdtime\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"stroke_threshold\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }, "
                     "    \"tracking_noise\": {"
                     "        \"base_value\": 0.0, "
                     "        \"inputs\": {}"
                     "    }"
                     "}, "
                     "\"version\": 3"
                  "}";
#define	MAX_MOTION_LENGTH	1024
#define MAX_COMMAND_LENGTH      1024
#define MAX_IMG_LENGTH          1024000

struct stroke {
    int tag;
    int length;
    int width;
    int height;
    struct motion {
        double x;
        double y;
        double pressure;
        double xtilt;
        double ytilt;
        unsigned int   time;
    }motions[MAX_MOTION_LENGTH];
};

struct command {
    int   tag;
    char  buf[MAX_COMMAND_LENGTH];
};
    
struct img {
    int    tag;
    int    x;
    int    y;
    int    size;
    char   buf[MAX_IMG_LENGTH];
};

struct PiBoardApp {
        GtkApplication *app;
	MyPaintResizableTiledSurface *surface;
	MyPaintBrush                 *brush;
	int	                     *nn_sockets;
        char                         **followers;
        char                         *output;
        char                         *channel;
        struct stroke                stroke;
        FILE                         *saved;
};

struct PiBoardApp	piboard;

static gboolean
button_press_event_cb (GtkWidget *widget,
                       GdkEventButton *event,
                       gpointer        data)
{
  // 获取并释放光标资源
  GdkCursor *cursor = gdk_window_get_cursor (gtk_widget_get_window(widget));
  if (cursor)
  {
    // 绘制图像到surface
    GdkPixbuf *pix = (GdkPixbuf *)g_object_get_data(G_OBJECT(cursor),
                                                    "Pixbuf");
    if (pix)
    {
      const guint8 *pixels = gdk_pixbuf_read_pixels (pix);
      int n_channels = gdk_pixbuf_get_n_channels (pix);
      int rowstride = gdk_pixbuf_get_rowstride (pix);
    
      int width = mypaint_resizable_tiled_surface_get_width (piboard.surface);
      if (width > gdk_pixbuf_get_width(pix))
        width =  gdk_pixbuf_get_width(pix);
      int height = mypaint_resizable_tiled_surface_get_height (piboard.surface);
      if (height > gdk_pixbuf_get_height(pix))
        height=  gdk_pixbuf_get_height(pix);
    
      int tile_size = MYPAINT_TILE_SIZE;
      int number_of_tile_rows = mypaint_resizable_tiled_surface_number_of_tile_rows (piboard.surface);
      int tiles_per_rows = mypaint_resizable_tiled_surface_tiles_per_rows (piboard.surface);
      MyPaintTiledSurface *surface = (MyPaintTiledSurface *)piboard.surface;
    
      int min_tx = floor(event->x / tile_size);
      int max_tx = ceil((event->x + width) / tile_size);
      int min_ty = floor(event->y / tile_size);
      int max_ty = ceil((event->y + height) / tile_size);
      for (int tx = min_tx; tx < max_tx; tx++)
      {

        for (int ty = min_ty; ty < max_ty; ty++)
        {
          int min_x = tx > min_tx ? 0 : (int)event->x % tile_size;
          int max_x = tx < max_tx - 1 ? tile_size : (width + (int)event->x) % tile_size;
          int min_y = ty > min_ty ? 0 : (int)event->y % tile_size;
          int max_y = ty < max_ty - 1 ? tile_size : (height + (int)event->y) % tile_size;

          MyPaintTileRequest request;
          mypaint_tile_request_init(&request, 0, tx, ty, FALSE);
          mypaint_tiled_surface_tile_request_start(surface, &request);
    
          for (int y = min_y; y < max_y; y++)
          {
            for (int x = min_x; x < max_x; x++)
            {
              int offset_x = tx * tile_size + x - event->x;
              int offset_y = ty * tile_size + y - event->y;
              int offset = offset_y * rowstride + offset_x * n_channels;
              request.buffer[(tile_size * y + x) * 4]  = pixels[offset] << 7;
              request.buffer[(tile_size * y + x) * 4 + 1]  = pixels[offset + 1] << 7;
              request.buffer[(tile_size * y + x) * 4 + 2]  = pixels[offset + 2] << 7;
            }
          }
          mypaint_tiled_surface_tile_request_end(surface, &request);
        }
      }
    
      // 绘制到屏幕
      surface->dirty_bbox.x = event->x; 
      surface->dirty_bbox.y = event->y; 
      surface->dirty_bbox.width = width; 
      surface->dirty_bbox.height= height; 
      gtk_widget_queue_draw_area (widget, event->x, event->y, width, height);

      // 发送到远程进行绘制
      struct img img;
      img.tag = 0x03;
      img.x = event->x;
      img.y = event->y;
      GError *error = NULL;
      gchar *buf = NULL;
      gdk_pixbuf_save_to_buffer (pix,
                                 &buf,
                                 (gsize *)&img.size,
                                 "png",
                                 &error,
                                 NULL);
      if (!error
      && buf
      && piboard.followers)
      {
        memcpy (img.buf, buf, img.size);
        for (int i = 0; piboard.nn_sockets[i] >= 0; i++)
        {
          nn_send (piboard.nn_sockets[i],
                   &img,
                   sizeof (struct img) - (MAX_IMG_LENGTH - img.size),
                   NN_DONTWAIT);
        }

        // 录制，需要转码
#if 0
        if (piboard.saved)
        {
          fprintf (piboard.saved, "i %s %u\n",
                   event->keyval,
                   event->time);
        }
#endif
      }
      if (buf)
        free (buf);
      g_object_unref(pix);
    }
// 释放光标资源
    gdk_window_set_cursor (gtk_widget_get_window(widget),
                           NULL);
    g_object_unref(cursor);
  }
  return FALSE;
}

static gboolean
button_scroll_event_cb (GtkWidget      *widget,
                        GdkEventScroll *event,
                        gpointer        data)
{
  // 获取并释放光标资源
  GdkCursor *cursor = gdk_window_get_cursor (gtk_widget_get_window(widget));
  if (cursor)
  {
    GdkPixbuf *src= (GdkPixbuf *)g_object_get_data(G_OBJECT(cursor),
                                                    "Pixbuf");
    GdkPixbuf *dest = NULL;
    if (src)
    {
      switch (event->direction)
      {
        case  GDK_SCROLL_UP:
            dest = gdk_pixbuf_scale_simple (src,
                                            (double)gdk_pixbuf_get_width(src) * 1.1,
                                            (double)gdk_pixbuf_get_height(src) * 1.1,
                                            GDK_INTERP_NEAREST);
            break;
        case  GDK_SCROLL_DOWN:
            dest = gdk_pixbuf_scale_simple (src,
                                            (double)gdk_pixbuf_get_width(src) / 1.1,
                                            (double)gdk_pixbuf_get_height(src) / 1.1,
                                            GDK_INTERP_NEAREST);
            break;
        default:
            break;
      }
    }

    if (dest)
    {
      g_object_unref(cursor);
      g_object_unref(src);
      cursor = gdk_cursor_new_from_pixbuf (gdk_display_get_default(),
                                           dest,
                                           0,
                                           0);

      g_object_set_data(G_OBJECT(cursor),
                        "Pixbuf",
                        dest);
      gdk_window_set_cursor (gtk_widget_get_window(widget),
                             cursor);
    }
  }
  return FALSE;
}

static gboolean
screen_draw (GtkWidget *widget,
             cairo_t *cr,
             gpointer   data)
{
  MyPaintTiledSurface *surface = (MyPaintTiledSurface *)piboard.surface;

  int width = mypaint_resizable_tiled_surface_get_width (piboard.surface);
  int height = mypaint_resizable_tiled_surface_get_height (piboard.surface);

  int tile_size = MYPAINT_TILE_SIZE;
  int number_of_tile_rows = mypaint_resizable_tiled_surface_number_of_tile_rows (piboard.surface);
  int tiles_per_rows = mypaint_resizable_tiled_surface_tiles_per_rows (piboard.surface);


  for (int tx = floor((double)surface->dirty_bbox.x  / tile_size); tx < ceil((double)(surface->dirty_bbox.x + surface->dirty_bbox.width) / tile_size); tx++)
  {
    for (int ty = floor((double)surface->dirty_bbox.y / tile_size); ty < ceil((double)(surface->dirty_bbox.y + surface->dirty_bbox.height) / tile_size); ty++)
    {
      int max_x = tx < tiles_per_rows - 1 || width % tile_size == 0 ? tile_size : width % tile_size;
      int max_y = ty < number_of_tile_rows - 1 || height % tile_size == 0 ? tile_size : height % tile_size;
      MyPaintTileRequest request;
      mypaint_tile_request_init(&request, 0, tx, ty, TRUE);
      mypaint_tiled_surface_tile_request_start(surface, &request);
      for (int y = 0; y < max_y; y++) {
        int yy = ty * tile_size + y;
        for (int x = 0; x < max_x; x++) {
          int xx = tx * tile_size + x;
          int offset = tile_size * y + x;
          uint16_t r = request.buffer[offset * 4] & 0x7FFF;
          uint16_t g = request.buffer[offset * 4 + 1] & 0x7FFF;
          uint16_t b = request.buffer[offset * 4 + 2] & 0x7FFF;
          float rr = (float)r / 0x7FFF;
          float gg = (float)g / 0x7FFF;
          float bb = (float)b / 0x7FFF;
          
          cairo_set_source_rgb(cr, rr, gg, bb);
          cairo_set_line_width (cr, 1);
          cairo_move_to(cr, xx, yy);
          cairo_line_to(cr, xx + 1, yy + 1);
          cairo_stroke(cr);
        }
      }
      mypaint_tiled_surface_tile_request_end(surface, &request);
    }
  }
  surface->dirty_bbox.x = 0;
  surface->dirty_bbox.y = 0;
  surface->dirty_bbox.width = 0;
  surface->dirty_bbox.height = 0;
  return TRUE;
}

static void
brush_draw (GtkWidget *widget, struct stroke *stroke)
{
  if (!stroke->length)
    return;
  guint32 last = stroke->motions[0].time;
  double dtime;
  MyPaintRectangle roi;
  MyPaintSurface *surface;
  surface = (MyPaintSurface *)piboard.surface;
  mypaint_surface_begin_atomic(surface);
  mypaint_brush_reset (piboard.brush);
  mypaint_brush_new_stroke (piboard.brush);
  for (int i = 0; i < stroke->length; i++)  {
    dtime = (double)(stroke->motions[i].time - last) / 1000;
    last = stroke->motions[i].time;
    mypaint_brush_stroke_to (piboard.brush,
                            surface,
                            stroke->motions[i].x,
                            stroke->motions[i].y,
                            stroke->motions[i].pressure,
                            0.0,
                            0.0,
                            dtime);
  }
  mypaint_surface_end_atomic(surface, &roi);
  
  gtk_widget_queue_draw_area(widget, roi.x, roi.y, roi.width, roi.height);
}

static void
image_draw (GtkWidget *widget,
            const gchar *filename)
{
  GError *error = NULL;
  GdkPixbuf *pix = gdk_pixbuf_new_from_file (filename,
                                             &error);
  if (error)
  {
    fprintf (stderr, "图像文件打开失败: %s\n", filename);
    return;
  }

  // 将窗口光标设置为图象
  // 通过鼠标操作来定义图象的显示位置和缩放大小
  // 移动鼠标确定位置
  // 滚轮改变缩放大小
  // 绘制过程将由鼠标按下回调函数执行
  GdkCursor *cursor = gdk_cursor_new_from_pixbuf (gdk_display_get_default(),
                                                  pix,
                                                  0,
                                                  0);

  gdk_window_set_cursor (gtk_widget_get_window(widget),
                         cursor);
  g_object_set_data(G_OBJECT(cursor),
                    "Pixbuf",
                    pix);
}

static void 
clear_surface (GtkWidget *widget)
{
  mypaint_resizable_tiled_surface_clear (piboard.surface);
  gtk_widget_queue_draw (widget);
}

static gboolean
key_press_event_cb (GtkWidget *widget,
                    GdkEventKey *event,
                    gpointer data)
{
  MyPaintTiledSurface *surface;
  GtkWidget *dialog;

  switch (event->keyval)
  {
    case GDK_KEY_c:	// 清屏
         // 本地清屏
         clear_surface(widget);

         // 远程清屏
         if (piboard.followers)
         {
           struct command command;
           command.tag = 0x02;
           sprintf (command.buf, "systemctl --user restart piboard");
           for (int i = 0; piboard.nn_sockets[i] >= 0; i++)
           {
             nn_send (piboard.nn_sockets[i],
                      &command,
                      sizeof (struct command) - (MAX_COMMAND_LENGTH - strlen(command.buf)) + 1,
                      NN_DONTWAIT);
           }
         }

         // 录制
         if (piboard.saved)
           fprintf (piboard.saved, "K %d %u\n",
                    event->keyval,
                    event->time);
         break;
    case GDK_KEY_l:	// 强制刷新本地屏幕
         surface = (MyPaintTiledSurface *)piboard.surface;
         int width = mypaint_resizable_tiled_surface_get_width (piboard.surface);
         int height = mypaint_resizable_tiled_surface_get_height (piboard.surface);
         surface->dirty_bbox.y = 0; 
         surface->dirty_bbox.y = 0; 
         surface->dirty_bbox.width = width; 
         surface->dirty_bbox.height = height; 
         gtk_widget_queue_draw(widget);
         break;
    case GDK_KEY_i:	// 显示一副图像
         dialog = gtk_file_chooser_dialog_new ("Open File",
                                                NULL,
                                                GTK_FILE_CHOOSER_ACTION_OPEN,
                                                "_Cancel",
                                                GTK_RESPONSE_CANCEL,
                                                "_Open",
                                                GTK_RESPONSE_ACCEPT,
                                                NULL);
         gtk_dialog_run (GTK_DIALOG (dialog));
         char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
         gtk_widget_destroy (dialog);

         if (filename)
         {
           image_draw(widget,
                      filename);
           free (filename);
         }
         break;
    case GDK_KEY_q:
         // 退出远程
         if (piboard.followers)
         {
           struct command command;
           command.tag = 0x02;
           sprintf (command.buf, "systemctl --user stop mumble");
           if (piboard.channel)
             sprintf (command.buf, "%s@`systemd-escape %s`",
                      command.buf,
                      piboard.channel);
           for (int i = 0; piboard.nn_sockets[i] >= 0; i++)
           {
             nn_send (piboard.nn_sockets[i],
                      &command,
                      sizeof (struct command) - (MAX_COMMAND_LENGTH - strlen(command.buf)) + 1,
                      NN_DONTWAIT);
           }
         }

         // 录制
         if (piboard.saved)
           fprintf (piboard.saved, "K %d %u\n",
                    event->keyval,
                    event->time);

         sleep (1);  // 等待1秒钟，上述任务被完成
         g_application_quit(G_APPLICATION(piboard.app));
         break;
    default:
         break;
  }

  return TRUE;
}

static gboolean
motion_notify_event_cb (GtkWidget *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  if (event->state & GDK_BUTTON1_MASK
  && piboard.stroke.length < MAX_MOTION_LENGTH)
  {
    piboard.stroke.motions[piboard.stroke.length].x = event->x;
    piboard.stroke.motions[piboard.stroke.length].y = event->y;
    if (!gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &piboard.stroke.motions[piboard.stroke.length].pressure))
        piboard.stroke.motions[piboard.stroke.length].pressure = DEFAULT_PRESSURE;
    if (!gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_XTILT, &piboard.stroke.motions[piboard.stroke.length].xtilt))
        piboard.stroke.motions[piboard.stroke.length].xtilt = DEFAULT_XTILT;
    if (!gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_YTILT, &piboard.stroke.motions[piboard.stroke.length].ytilt))
        piboard.stroke.motions[piboard.stroke.length].ytilt = DEFAULT_YTILT;
    piboard.stroke.motions[piboard.stroke.length].time = event->time;

    piboard.stroke.length++;
  }

  return TRUE;
}

static gboolean
button_release_event_cb (GtkWidget *widget,
                       GdkEventButton *event,
                       gpointer        data)
{
  if (event->button == GDK_BUTTON_PRIMARY)
  {
    piboard.stroke.width = gtk_widget_get_allocated_width (widget);
    piboard.stroke.height = gtk_widget_get_allocated_height (widget);
    brush_draw (widget, &piboard.stroke);
    if (piboard.followers)
    {
      piboard.stroke.tag = 0x00;
      for (int i = 0; piboard.nn_sockets[i] >= 0; i++)
        nn_send (piboard.nn_sockets[i],
                 &piboard.stroke,
                 sizeof (struct stroke) - sizeof (struct motion) * (MAX_MOTION_LENGTH - piboard.stroke.length),
                 NN_DONTWAIT);
      if (piboard.saved)
      {
        for (int i = 0; i < piboard.stroke.length; i++)
          fprintf (piboard.saved, "M %.02f %.02f %.02f %u\n",
                   piboard.stroke.motions[i].x,
                   piboard.stroke.motions[i].y,
                   piboard.stroke.motions[i].pressure,
                   piboard.stroke.motions[i].time);
        fprintf (piboard.saved, "R %d %d %u\n",
                 piboard.stroke.width,
                 piboard.stroke.height,
                 event->time);
      }
    }
    piboard.stroke.length = 0;
  }
  return TRUE;
}

static void
window_close (GtkWidget *widget,
              gpointer   data)
{
  if (piboard.followers)
    g_strfreev(piboard.followers);
  if (piboard.saved)
    fclose (piboard.saved);
  if (piboard.surface)
    mypaint_surface_unref((MyPaintSurface *)piboard.surface);
  if (piboard.brush)
    mypaint_brush_unref(piboard.brush);
  for(int i = 0; piboard.nn_sockets[i] >= 0; i++)
    nn_close(piboard.nn_sockets[i]);
  if (piboard.nn_sockets)
    free (piboard.nn_sockets);
  g_application_quit(G_APPLICATION(piboard.app));
}

static gboolean
configure_event_cb (GtkWidget         *widget,
                    GdkEventConfigure *event,
                    gpointer           data)
{
  if (piboard.surface)
  {
    mypaint_surface_unref ((MyPaintSurface *)piboard.surface);
  }
  piboard.surface = mypaint_resizable_tiled_surface_new(gtk_widget_get_allocated_width (widget),
                                                        gtk_widget_get_allocated_height(widget));


  if (!piboard.brush)
  {
    piboard.brush = mypaint_brush_new();

    if (mypaint_brush_from_string(piboard.brush, brush) != TRUE)
    {
      mypaint_brush_from_defaults(piboard.brush);
      mypaint_brush_set_base_value(piboard.brush, MYPAINT_BRUSH_SETTING_COLOR_H, 0.0);
      mypaint_brush_set_base_value(piboard.brush, MYPAINT_BRUSH_SETTING_COLOR_S, 1.0);
      mypaint_brush_set_base_value(piboard.brush, MYPAINT_BRUSH_SETTING_COLOR_V, 0.0);
    }
  }
  return TRUE;
}

static gboolean
nn_sub(gpointer data)
{
  int bytes;
  void *msg = NULL;

  for (int i = 0; piboard.nn_sockets[i] >= 0; i++)
  {
    bytes = nn_recv (piboard.nn_sockets[i], &msg, NN_MSG, NN_DONTWAIT);
    if (bytes <=  0)
      continue;

    GtkWidget        *widget;
    struct stroke    *stroke;
    struct command   *command;
    struct img       *img;

    int *tag = (int *)msg;
    switch (*tag)
    {
        case 0x00:
            widget = (GtkWidget *)data;
            stroke = (struct stroke *)msg;
            for (int i = 0; i < stroke->length; i++)
            {
              stroke->motions[i].x *= (double)gtk_widget_get_allocated_width (widget) / stroke->width;
              stroke->motions[i].y *= (double)gtk_widget_get_allocated_height (widget) / stroke->height;
            }
            brush_draw (widget, stroke);
            break;
        case 0x02:
            command = (struct command *)msg;
            system(command->buf);
            break;
        case 0x03:
            widget = (GtkWidget *)data;
            img = (struct img *)msg;
            char filename[128];
            sprintf (filename, "/tmp/temp.XXXXXX");
            int fd = mkstemp (filename);
            if (fd > 0)
            {
              write(fd, img->buf, img->size);
              close (fd);
              printf ("save to file: %s\n", filename);
              image_draw (widget,
                          filename);
              GdkEventButton* event = (GdkEventButton *)gdk_event_new(GDK_BUTTON_PRESS);
              event->window = gtk_widget_get_window(widget);
              g_object_ref(event->window);
              event->time = GDK_CURRENT_TIME;
              event->x = img->x;
              event->y = img->y;
              gtk_main_do_event ((GdkEvent *)event);
            }
            break;
        default:
            break;
    }
    nn_freemsg (msg);
  }
  return TRUE;
}

static void
app_activate (GtkApplication *app,
              gpointer        data)
{
  GtkWidget *window;
  window = gtk_application_window_new (app);

  g_signal_connect (window,
                    "destroy",
                    G_CALLBACK (window_close),
                    NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  GtkWidget *drawing_area = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (window), drawing_area);

  /* Signals used to handle the backing surface */
  g_signal_connect (drawing_area,
                    "draw",
                    G_CALLBACK (screen_draw), NULL);

  g_signal_connect (drawing_area, "configure-event",
                    G_CALLBACK (configure_event_cb),
                    NULL);
  g_signal_connect (drawing_area,
                   "button-press-event",
                   G_CALLBACK (button_press_event_cb),
                   NULL);
  g_signal_connect(drawing_area,
                   "scroll-event", 
                   G_CALLBACK(button_scroll_event_cb),
                   NULL);
  g_signal_connect (drawing_area, "button-release-event",
                    G_CALLBACK (button_release_event_cb), NULL);
  g_signal_connect (drawing_area, "key-press-event",
                    G_CALLBACK (key_press_event_cb), NULL);
  g_signal_connect (drawing_area, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);

  gtk_widget_set_can_focus (drawing_area, TRUE);
  gtk_widget_add_events (drawing_area, 
                         GDK_KEY_PRESS_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_POINTER_MOTION_MASK
                         | GDK_SCROLL_MASK);
  if (piboard.followers)
  {
    for(int i = 0; piboard.followers[i]; i++)
    {
      piboard.nn_sockets = (int *)realloc (piboard.nn_sockets,
                                           sizeof (int) * (i + 2));
      piboard.nn_sockets[i] = nn_socket (AF_SP, NN_PAIR);
      char url[64];
      sprintf (url, "tcp://%s:7789", piboard.followers[i]);
      nn_connect (piboard.nn_sockets[i], url);
      piboard.nn_sockets[i + 1] = -1;
    }
    // 等待连接建立
    sleep(1);

    struct command command;
    command.tag = 0x02;
    if (piboard.channel)
      sprintf (command.buf, "systemctl --user start mumble@`systemd-escape %s`", piboard.channel);
    else
      sprintf (command.buf, "systemctl --user start mumble");
    for (int i = 0; piboard.nn_sockets[i] >= 0; i++)
      nn_send (piboard.nn_sockets[i],
               &command,
               sizeof (struct command) - (MAX_COMMAND_LENGTH - strlen(command.buf)) + 1,
               NN_DONTWAIT);
    sleep (1);
    sprintf (command.buf, "systemctl --user restart piboard");
    for (int i = 0; piboard.nn_sockets[i] >= 0; i++)
      nn_send (piboard.nn_sockets[i],
               &command,
               sizeof (struct command) - (MAX_COMMAND_LENGTH - strlen(command.buf)) + 1,
               NN_DONTWAIT);

    if (piboard.output)
      piboard.saved = fopen (piboard.output, "w");
    if (piboard.saved)
      fprintf (piboard.saved, "K %d %u\n",
               GDK_KEY_c,
               (unsigned int)g_get_monotonic_time() / 1000);
  }
  else
  {
    piboard.nn_sockets = (int *)malloc (sizeof (int) * 2);
    piboard.nn_sockets[0] = nn_socket (AF_SP, NN_PAIR);
    nn_bind (piboard.nn_sockets[0], "tcp://*:7789");
    g_timeout_add (10, nn_sub, drawing_area);
    piboard.nn_sockets[1] = -1;
  }

  gtk_window_fullscreen(GTK_WINDOW(window));
  gtk_window_set_keep_above (GTK_WINDOW(window), TRUE);
  
  gtk_widget_show_all (window);
  gtk_window_set_decorated (GTK_WINDOW(window),
                            FALSE);
}

int
main (int    argc,
      char **argv)
{
  int status;

  memset (&piboard, 0, sizeof (struct PiBoardApp));

  piboard.app = gtk_application_new ("com.pi-classroom.piboard",
                                     0);

  g_signal_connect (piboard.app,
                    "activate",
                    G_CALLBACK (app_activate), NULL);

  const GOptionEntry options[] = {
                                   {
                                     .long_name       = "follow",
                                     .short_name      = 'f',
                                     .flags           = G_OPTION_FLAG_NONE,
                                     .arg             = G_OPTION_ARG_STRING_ARRAY,
                                     .arg_data        = &piboard.followers,
                                     .description     = "远端白板",
                                     .arg_description = NULL,
                                   },
                                   {
                                     .long_name       = "output",
                                     .short_name      = 'o',
                                     .flags           = G_OPTION_FLAG_NONE,
                                     .arg             = G_OPTION_ARG_FILENAME,
                                     .arg_data        = &piboard.output,
                                     .description     = "保存文件名",
                                     .arg_description = NULL,
                                   },
                                   {
                                     .long_name       = "channel",
                                     .short_name      = 'c',
                                     .flags           = G_OPTION_FLAG_NONE,
                                     .arg             = G_OPTION_ARG_STRING,
                                     .arg_data        = &piboard.channel,
                                     .description     = "频道名称",
                                     .arg_description = NULL,
                                   },
                                   {
                                     NULL,
                                   }
                                 };
  g_application_add_main_option_entries (G_APPLICATION (piboard.app),
                                         options);

  status = g_application_run (G_APPLICATION (piboard.app),
                              argc,
                              argv);
  g_object_unref (piboard.app);

  return status;
}
