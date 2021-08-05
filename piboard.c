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
#include "shl_array.h"

#define DEFAULT_BRUSH	"deevad/liner.myb"

typedef enum {
    MOTION,
    BUTTON1_PRESSED,
    BUTTON2_PRESSED,
    BUTTON1_RELEASED,
    BUTTON2_RELEASED
} PUBLISH_EVENT_TYPE;

struct PublishEvent {
    PUBLISH_EVENT_TYPE type;
    unsigned short x;
    unsigned short y;
    unsigned char pressure;
    unsigned char xtilt;
    unsigned char ytilt;
    unsigned short width;
    unsigned short height;
    int      button;
    unsigned int   time;
};

struct PiBoardApp {
	MyPaintResizableTiledSurface *surface;
	MyPaintBrush                 *brush;
	int	                     nn_socket;
        char                         *publisher;
        struct shl_array             *motions;
};

struct PiBoardApp	piboard;

static gboolean
screen_draw (GtkWidget *widget,
         cairo_t   *cr, gpointer   data)
{
  const int width = mypaint_resizable_tiled_surface_get_width (piboard.surface);
  const int height = mypaint_resizable_tiled_surface_get_height (piboard.surface);

  const int tile_size = MYPAINT_TILE_SIZE;
  const int number_of_tile_rows = mypaint_resizable_tiled_surface_number_of_tile_rows (piboard.surface);
  const int tiles_per_rows = mypaint_resizable_tiled_surface_tiles_per_rows (piboard.surface);
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
  piboard.motions->length = 0;
  return G_SOURCE_CONTINUE;
}

static void
brush_draw (GtkWidget *widget)
{
  int size = shl_array_get_length (piboard.motions);
  if (!size)
    return;
  struct PublishEvent *pe = SHL_ARRAY_AT(piboard.motions, struct PublishEvent, 0);
  guint32 last = pe->time;
  double dtime;
  MyPaintRectangle roi;
  MyPaintSurface *surface;
  surface = (MyPaintSurface *)piboard.surface;
  mypaint_surface_begin_atomic(surface);
  mypaint_brush_reset (piboard.brush);
  mypaint_brush_new_stroke (piboard.brush);
  for (int i = 0; i < size; i++)
  {
    pe = SHL_ARRAY_AT(piboard.motions, struct PublishEvent, i);
    dtime = (double)(pe->time - last) / 1000;
    last = pe->time;

    mypaint_brush_stroke_to ( piboard.brush,
                            surface,
                            pe->x,
                            pe->y,
                            (double)pe->pressure / 10,
                            (double)pe->xtilt / 10,
                            (double)pe->ytilt / 10,
                            dtime);
  }
  mypaint_surface_end_atomic(surface, &roi);
  
  gtk_widget_queue_draw_area (widget, roi.x, roi.y, roi.width, roi.height);

  //gdk_window_process_updates (widget->window, TRUE);
}

static void 
clear_surface ()
{
  mypaint_resizable_tiled_surface_clear (piboard.surface);
}

static gboolean
motion_notify_event_cb (GtkWidget *widget,
                        GdkEventMotion *event,
                        gpointer        data)
{
  struct PublishEvent *pe;
  if (data)
    pe = (struct PublishEvent *)data;
  else
  {
    pe = (struct PublishEvent *)malloc (sizeof (struct PublishEvent) );
    pe->type = MOTION;
    pe->x = event->x;
    pe->y = event->y;
    gdouble pressure, xtilt, ytilt;
    pe->pressure = gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_PRESSURE, &pressure) ? pressure * 10 : 10;
    pe->xtilt = gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_XTILT, &xtilt) ? xtilt * 10 : 0;
    pe->ytilt = gdk_event_get_axis ((GdkEvent *)event, GDK_AXIS_YTILT, &ytilt) ? ytilt * 10 : 0;
    pe->width = gtk_widget_get_allocated_width (widget);
    pe->height = gtk_widget_get_allocated_height (widget);
    pe->time = event->time;
  }
  shl_array_push (piboard.motions, pe);
  if (!piboard.publisher)
    nn_send (piboard.nn_socket, pe, sizeof (struct PublishEvent), NN_DONTWAIT);

  if (!data)
    free (pe);

  return TRUE;
}

static gboolean
button_press_event_cb (GtkWidget *widget,
                       GdkEventButton *event,
                       gpointer        data)
{
  struct PublishEvent pe;
  switch (event->button)
  {
    case GDK_BUTTON_PRIMARY:
      pe.type = BUTTON1_PRESSED;
      g_signal_connect (widget, "motion-notify-event",
                    G_CALLBACK (motion_notify_event_cb), NULL);
      break;
    case GDK_BUTTON_SECONDARY:
      pe.type = BUTTON2_PRESSED;
      clear_surface();
      gtk_widget_queue_draw (widget);
      break;
  }
  if (!piboard.publisher)
    nn_send (piboard.nn_socket, &pe, sizeof (struct PublishEvent), NN_DONTWAIT);

  return TRUE;
}

static gboolean
button_release_event_cb (GtkWidget *widget,
                       GdkEventButton *event,
                       gpointer        data)
{
  struct PublishEvent pe;
  if (event->state & GDK_BUTTON1_MASK)
  {
    g_signal_handlers_disconnect_by_func (widget, G_CALLBACK (motion_notify_event_cb), NULL);
    brush_draw (widget);
    pe.type = BUTTON1_RELEASED;
  }
  else if (event->state & GDK_BUTTON2_MASK)
    pe.type = BUTTON2_RELEASED;
  else
    return TRUE;

  if (!piboard.publisher)
    nn_send (piboard.nn_socket, &pe, sizeof (struct PublishEvent), NN_DONTWAIT);
  return TRUE;
}

static void
close_window (GtkWidget *widget,
              gpointer   data)
{
  if (piboard.surface)
    mypaint_surface_unref((MyPaintSurface *)piboard.surface);
  if (piboard.brush)
    mypaint_brush_unref(piboard.brush);
  shl_array_free (piboard.motions);
  nn_close(piboard.nn_socket);
  g_application_quit(G_APPLICATION(data));
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
nn_sub (gpointer user_data)
{
    int bytes;
    void *msg = NULL;
    bytes = nn_recv (piboard.nn_socket, &msg, NN_MSG, NN_DONTWAIT);
    if (bytes == sizeof(struct PublishEvent) )
    {
        struct PublishEvent *event = (struct PublishEvent *)msg;
        GdkEventButton      button;
        switch (event->type)
        {
            case MOTION:
                event->x *= gtk_widget_get_allocated_width ((GtkWidget *)user_data) / event->width;
                event->y *= gtk_widget_get_allocated_height ((GtkWidget *)user_data) / event->height;
                motion_notify_event_cb ((GtkWidget *)user_data,
                                        NULL,
                                        event);
                break;
            case BUTTON1_PRESSED:
                button.button = GDK_BUTTON_PRIMARY;
                button_press_event_cb ((GtkWidget *)user_data,
                                       &button,
                                       NULL);
                break;
            case BUTTON2_PRESSED:
                button.button = GDK_BUTTON_SECONDARY;
                button_press_event_cb ((GtkWidget *)user_data,
                                       &button,
                                       NULL);
                break;
            case BUTTON1_RELEASED:
                button.state |= GDK_BUTTON1_MASK;
                button_release_event_cb ((GtkWidget *)user_data,
                                       &button,
                                       NULL);
                break;
            case BUTTON2_RELEASED:
                button.state |= GDK_BUTTON2_MASK;
                button_release_event_cb ((GtkWidget *)user_data,
                                       &button,
                                       NULL);
                break;
            default:
                break;
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
  GtkWidget *frame;
  GtkWidget *drawing_area;

  window = gtk_application_window_new (app);
  if (piboard.publisher)
    gtk_window_set_title (GTK_WINDOW (window), "PiBoard-Subcriber");
  else
    gtk_window_set_title (GTK_WINDOW (window), "PiBoard-Publisher");
  gtk_widget_set_size_request (window, 300, 200);

  g_signal_connect (window, "destroy", G_CALLBACK (close_window), user_data);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (window), frame);

  drawing_area = gtk_drawing_area_new ();

  gtk_container_add (GTK_CONTAINER (frame), drawing_area);

  /* Signals used to handle the backing surface */
  g_signal_connect (drawing_area, "configure-event",
                    G_CALLBACK (configure_event_cb), NULL);

  g_signal_connect (drawing_area, "draw",
                    G_CALLBACK (screen_draw), NULL);
  g_signal_connect (drawing_area, "button-press-event",
                    G_CALLBACK (button_press_event_cb), NULL);
  g_signal_connect (drawing_area, "button-release-event",
                    G_CALLBACK (button_release_event_cb), NULL);

  gtk_widget_set_events (drawing_area, gtk_widget_get_events (drawing_area)
                                     | GDK_BUTTON_PRESS_MASK
                                     | GDK_BUTTON_RELEASE_MASK
                                     | GDK_POINTER_MOTION_MASK);
  if (!piboard.publisher)
  {
    piboard.nn_socket = nn_socket (AF_SP, NN_PUB);
    nn_bind (piboard.nn_socket, "tcp://*:7789");
  }
  else
  {
    piboard.nn_socket = nn_socket (AF_SP, NN_SUB);
    nn_setsockopt(piboard.nn_socket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
    char url[100];
    memset (url, 0, 100);
    sprintf (url, "tcp://%s:7789", piboard.publisher);
    nn_connect (piboard.nn_socket, url);
    g_timeout_add (10, nn_sub, drawing_area);
  }
  
  gtk_widget_show_all (window);
  gtk_window_present(GTK_WINDOW(window));
}

int
main (int    argc,
      char **argv)
{
  GtkApplication *app;
  int status;

  memset (&piboard, 0, sizeof (struct PiBoardApp));
  shl_array_new (&piboard.motions, sizeof (struct PublishEvent), 1024);

  FILE *fp;
  fp = fopen ("/etc/piboard.conf", "r");
  if (fp)
  {
    char  buf[1024];
    char  *p;
    while (fgets (buf, 1024, fp) != NULL)
    {
        if (buf[0] == '#')
            continue;
        // trim the string
        for (int i = strlen(buf) - 1; i >= 0; i--)
            if (isspace (buf[i]))
                buf[i] = '\0';
        p = buf;
        for (int i = 0; i < strlen(buf); i++)
            if (isspace (buf[i]))
                p++;
        if (strlen(p) == 0)
            continue;
        piboard.publisher = strdup (p);
        // only one line is ok
        break;
    }
    fclose (fp);
  }

  char	str[128];
  memset (str, 0, 128);
  sprintf (str, "com.pi-classroom.piboard_%04d", getpid() );
  app = gtk_application_new (str, G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "activate", G_CALLBACK (activate), app);

  status = g_application_run (G_APPLICATION (app), 0, NULL);
  g_object_unref (app);

  return status;
}
