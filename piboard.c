/*
 *   程序名：piboard.c
 *     版本：1.0
 *     作者：成建文 <chengjianwen@gmail.com>
 */

#include <gtk/gtk.h>
#include <mypaint-brush.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <ctype.h>
#include <math.h>
#include "mypaint-resizable-tiled-surface.h"

#pragma pack(4)

#define DEFAULT_BRUSH	"deevad/liner.myb"
#define	MOTION_LENGTH	1024

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
    }motions[MOTION_LENGTH];
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
        char                         *publisher;
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
      const int max_x = tx < tiles_per_rows - 1 || width % tile_size == 0 ? tile_size : width % tile_size;
      const int max_y = ty < number_of_tile_rows - 1 || height % tile_size == 0 ? tile_size : height % tile_size;
      MyPaintTileRequest request;
      mypaint_tile_request_init(&request, 0, tx, ty, TRUE);
      mypaint_tiled_surface_tile_request_start(surface, &request);
      for (int y = 0; y < max_y; y++) {
        const int yy = ty * tile_size + y;
        for (int x = 0; x < max_x; x++) {
          const int xx = tx * tile_size + x;
          const int offset = tile_size * y + x;
          const uint16_t r = (request.buffer[offset * 4] & 0xF800) >> 11;
          const uint16_t g = (request.buffer[offset * 4] & 0x07E0) >> 5;
          const uint16_t b = request.buffer[offset * 4] & 0x1F;
          cairo_set_source_rgb(cr, (double)r / 0x1F, (double)g / 0x3F, (double)b / 0x1F);
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
  for (int i = 1; i < stroke->length; i++)
  {
    dtime = (double)(stroke->motions[i].time - last) / 1000;
    last = stroke->motions[i].time;

    mypaint_brush_stroke_to (piboard.brush,
                            surface,
                            stroke->motions[i].x,
                            stroke->motions[i].y,
                            stroke->motions[i].pressure,
                            stroke->motions[i].xtilt,
                            stroke->motions[i].ytilt,
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
  if (!piboard.publisher)
  {
    struct key key;
    key.tag.tag = 0x01;
    key.keyval = event->keyval;
    nn_send (piboard.nn_socket, &key, sizeof (struct key), NN_DONTWAIT);
    fprintf (piboard.saved, "K %d %u\n",
             event->keyval,
             event->time);
    fflush (piboard.saved);
  }
  return TRUE;
}

static gboolean
motion_notify_event_cb (GtkWidget *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  if (event->state & GDK_BUTTON1_MASK)
  {
    piboard.stroke.motions[piboard.stroke.length].x = event->x;
    piboard.stroke.motions[piboard.stroke.length].y = event->y;
    if (!gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &piboard.stroke.motions[piboard.stroke.length].pressure))
        piboard.stroke.motions[piboard.stroke.length].pressure = 1.0;
    if (!gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &piboard.stroke.motions[piboard.stroke.length].xtilt))
        piboard.stroke.motions[piboard.stroke.length].xtilt = 0.0;
    if (!gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &piboard.stroke.motions[piboard.stroke.length].ytilt))
        piboard.stroke.motions[piboard.stroke.length].ytilt = 0.0;
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
    brush_draw (widget, &piboard.stroke);
    if (!piboard.publisher)
    {
      piboard.stroke.tag.tag = 0x00;
      piboard.stroke.width = gtk_widget_get_allocated_width (widget);
      piboard.stroke.height = gtk_widget_get_allocated_height (widget);
      nn_send (piboard.nn_socket, &piboard.stroke, sizeof (struct stroke) - sizeof (struct motion) * (MOTION_LENGTH - piboard.stroke.length), NN_DONTWAIT);
      for (int i = 0; i < piboard.stroke.length; i++)
        fprintf (piboard.saved, "M %.02f %.02f %.02f %.02f %.02f %u\n",
                 piboard.stroke.motions[i].x,
                 piboard.stroke.motions[i].y,
                 piboard.stroke.motions[i].pressure,
                 piboard.stroke.motions[i].xtilt,
                 piboard.stroke.motions[i].ytilt,
                 piboard.stroke.motions[i].time);
      fprintf (piboard.saved, "R %d %d %u\n",
               piboard.stroke.width,
               piboard.stroke.height,
               event->time);
      fflush (piboard.saved);
    }
    piboard.stroke.length = 0;
  }
  return TRUE;
}

static void
close_window (GtkWidget *widget,
              gpointer   data)
{
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
    gboolean load_brush = FALSE;
    piboard.brush = mypaint_brush_new();

    char	path[1024];
    memset (path, 0, 1024);
    sprintf (path, "%s/.brushes/%s", getenv("HOME"), DEFAULT_BRUSH);
    FILE	*fp = fopen (path, "r");
    if (fp)
    {
      fseek (fp, 0 , SEEK_END);
      int	size = ftell (fp);
      rewind (fp);
      char *buffer = (char *)malloc (size);
      memset (buffer, 0, size);
      if ((fread (buffer, 1, size, fp)) == size)
      {
        if (mypaint_brush_from_string(piboard.brush, buffer) == TRUE)
        {
          load_brush = TRUE;
        }
      }
      free (buffer);
      fclose (fp);
    }

    if (!load_brush)
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
activate (GtkApplication *app,
          gpointer        user_data)
{
  GtkWidget *window;
  window = gtk_application_window_new (app);
  if (piboard.publisher)
    gtk_window_set_title (GTK_WINDOW (window), "PiBoard-Subcriber");
  else
    gtk_window_set_title (GTK_WINDOW (window), "PiBoard-Publisher");
  gtk_window_fullscreen(GTK_WINDOW(window));

  g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);

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
  if (!piboard.publisher)
  {
    piboard.nn_socket = nn_socket (AF_SP, NN_PUB);
    nn_bind (piboard.nn_socket, "tcp://*:7789");
    piboard.saved = fopen (".saved.txt", "w");
    fprintf (piboard.saved, "N %u\n",
             g_get_monotonic_time() / 1000);
  }
  else
  {
    piboard.nn_socket = nn_socket (AF_SP, NN_SUB);
    nn_setsockopt(piboard.nn_socket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
    g_timeout_add (10, nn_sub, drawing_area);
    char url[100];
    memset (url, 0, 100);
    sprintf (url, "tcp://%s:7789", piboard.publisher);
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

  FILE *fp;
  fp = fopen ("/etc/piboard.conf", "r");
  if (fp)
  {
    char  buf[1024];
    char  *p, *p1;
    while (fgets (buf, 1024, fp) != NULL)
    {
        p = buf;
        while (isspace((unsigned char)*p))
          p++;
        if (*p == 0 || *p == '#')
          continue;
        p1 = p + strlen(p) - 1;
        while (p1 > p && isspace((unsigned char)*p1))
          p1--;
        piboard.publisher = strndup (p, p1 - p + 1);
        // only one line
        break;
    }
    fclose (fp);
  }

  char	str[128];
  memset (str, 0, 128);
  sprintf (str, "com.pi-classroom.piboard_%04d", getpid() );
  piboard.app = gtk_application_new (str, G_APPLICATION_FLAGS_NONE);

  g_signal_connect (piboard.app, "activate", G_CALLBACK (activate), NULL);

  status = g_application_run (G_APPLICATION (piboard.app), 0, NULL);
  g_object_unref (piboard.app);

  return status;
}
