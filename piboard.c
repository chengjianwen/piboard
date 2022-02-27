/*
 *   程序名：piboard.c
 *     作者：成建文 <chengjianwen@gmail.com>
 *   修改日志: 
 *            2021-12-16	内嵌画笔kabura.myb到源代码
 */

#include <gtk/gtk.h>
#include <mypaint-brush.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <ctype.h>
#include <math.h>
#include "mypaint-resizable-tiled-surface.h"

#pragma pack(4)

// 缺省压感值
#define	DEFAULT_PRESSURE	0.5
// 缺省X倾角值
#define	DEFAULT_XTILT		0.0
// 缺省Y倾角值
#define	DEFAULT_YTILT		0.0
// 缺省保存文件名
#define DEFAULT_SAVETO          "output.txt"
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

struct tag {
    int tag;
};

struct stroke {
    struct tag tag;
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

struct key {
    struct tag tag;
    int keyval;   
};

struct PiBoardApp {
        GtkApplication *app;
	MyPaintResizableTiledSurface *surface;
	MyPaintBrush                 *brush;
	int	                     nn_socket;
        char                         *remote;
        char                         *saveto;
        struct stroke                stroke;
        FILE                         *saved;
};

struct PiBoardApp	piboard;

static gboolean
screen_draw (GtkWidget *widget,
             cairo_t *cr,
             gpointer   data)
{
  int width = mypaint_resizable_tiled_surface_get_width (piboard.surface);
  int height = mypaint_resizable_tiled_surface_get_height (piboard.surface);

  int tile_size = MYPAINT_TILE_SIZE;
  int number_of_tile_rows = mypaint_resizable_tiled_surface_number_of_tile_rows (piboard.surface);
  int tiles_per_rows = mypaint_resizable_tiled_surface_tiles_per_rows (piboard.surface);
  MyPaintTiledSurface *surface = (MyPaintTiledSurface *)piboard.surface;

  for (int tx = floor((double)surface->dirty_bbox.x / tile_size); tx < ceil((double)(surface->dirty_bbox.x + surface->dirty_bbox.width) / tile_size); tx++)
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
  return FALSE;
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
  switch (event->keyval)
  {
    case GDK_KEY_c:
         clear_surface(widget);
         break;
    case GDK_KEY_q:
         g_application_quit(G_APPLICATION(piboard.app));
         break;
    default:
         break;
  }
  if (!piboard.remote)
  {
    struct key key;
    key.tag.tag = 0x01;
    key.keyval = event->keyval;
    nn_send (piboard.nn_socket, &key, sizeof (struct key), NN_DONTWAIT);
    fprintf (piboard.saved, "K %d %u\n",
             event->keyval,
             event->time);
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
    if (!piboard.remote)
    {
      piboard.stroke.tag.tag = 0x00;
      nn_send (piboard.nn_socket, &piboard.stroke, sizeof (struct stroke) - sizeof (struct motion) * (MAX_MOTION_LENGTH - piboard.stroke.length), NN_DONTWAIT);
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
    piboard.stroke.length = 0;
  }
  return TRUE;
}

static void
window_close (GtkWidget *widget,
              gpointer   data)
{
  if (piboard.remote)
    free(piboard.remote);
  if (piboard.saved)
    fclose (piboard.saved);
  if (piboard.surface)
    mypaint_surface_unref((MyPaintSurface *)piboard.surface);
  if (piboard.brush)
    mypaint_brush_unref(piboard.brush);
  nn_close(piboard.nn_socket);
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
  GtkWidget *widget = data;
  void *msg = NULL;
  bytes = nn_recv (piboard.nn_socket, &msg, NN_MSG, NN_DONTWAIT);
  if (bytes > 0)
  {
    struct tag *tag = (struct tag *)msg;
    if (tag->tag == 0x00)
    {
      struct stroke *stroke = (struct stroke *)msg;
      for (int i = 0; i < stroke->length; i++)
      {
        stroke->motions[i].x *= (double)gtk_widget_get_allocated_width (widget) / stroke->width;
        stroke->motions[i].y *= (double)gtk_widget_get_allocated_height (widget) / stroke->height;
      }
      brush_draw (widget, stroke);
    }
    else if (tag->tag == 0x01)
    {
      struct key *key = (struct key *)msg;
      if (key->keyval == GDK_KEY_c)
        clear_surface(widget);
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
  if (piboard.remote)
    gtk_window_set_title (GTK_WINDOW (window), piboard.remote);
  gtk_window_fullscreen(GTK_WINDOW(window));

  g_signal_connect (window, "destroy", G_CALLBACK (window_close), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  GtkWidget *drawing_area = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (window), drawing_area);

  /* Signals used to handle the backing surface */
  g_signal_connect (drawing_area, "configure-event",
                    G_CALLBACK (configure_event_cb), NULL);

  g_signal_connect (drawing_area, "draw",
                    G_CALLBACK (screen_draw), NULL);
  g_signal_connect (drawing_area, "button-release-event",
                    G_CALLBACK (button_release_event_cb), NULL);
  g_signal_connect (drawing_area, "key-press-event",
                    G_CALLBACK (key_press_event_cb), NULL);
  g_signal_connect (drawing_area, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);

  gtk_widget_set_can_focus (drawing_area, TRUE);
  gtk_widget_add_events (drawing_area, 
                         GDK_KEY_PRESS_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
  if (!piboard.remote)
  {
    piboard.nn_socket = nn_socket (AF_SP, NN_PUB);
    nn_bind (piboard.nn_socket, "tcp://*:7789");
    piboard.saved = fopen (piboard.saveto ? piboard.saveto : DEFAULT_SAVETO, "w");
    fprintf (piboard.saved, "N %u\n",
             (unsigned int)g_get_monotonic_time() / 1000);
  }
  else
  {
    piboard.nn_socket = nn_socket (AF_SP, NN_SUB);
    nn_setsockopt(piboard.nn_socket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
    g_timeout_add (10, nn_sub, drawing_area);
    char url[64];
    sprintf (url, "tcp://%s:7789", piboard.remote);
    nn_connect (piboard.nn_socket, url);
  }

  gtk_widget_show_all (window);
  gtk_window_present(GTK_WINDOW(window));
}

int
main (int    argc,
      char **argv)
{
  int status;

  memset (&piboard, 0, sizeof (struct PiBoardApp));

  piboard.app = gtk_application_new ("com.pi-classroom.piboard",
                                     G_APPLICATION_FLAGS_NONE);

  g_signal_connect (piboard.app,
                    "activate",
                    G_CALLBACK (app_activate), NULL);

  const GOptionEntry options[] = {
                                   {
                                     .long_name       = "remote",
                                     .short_name      = 'r',
                                     .flags           = G_OPTION_FLAG_NONE,
                                     .arg             = G_OPTION_ARG_STRING,
                                     .arg_data        = &piboard.remote,
                                     .description     = "远程服务器地址",
                                     .arg_description = NULL,
                                   },
                                   {
                                     .long_name       = "saveto",
                                     .short_name      = 's',
                                     .flags           = G_OPTION_FLAG_NONE,
                                     .arg             = G_OPTION_ARG_FILENAME,
                                     .arg_data        = &piboard.saveto,
                                     .description     = "保存文件名，缺省为: output.txt",
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
