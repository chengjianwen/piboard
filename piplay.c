/*
 *   程序名：piplay.c
 *     版本：1.0
 *     作者：成建文 <chengjianwen@gmail.com>
 */

#include <gtk/gtk.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <ctype.h>
#include <math.h>

#pragma pack(4)

typedef enum {
    MOTION,
    BUTTON1_PRESSED,
    BUTTON2_PRESSED,
    BUTTON1_RELEASED,
    BUTTON2_RELEASED,
    KEY_PRESSED
} PUBLISH_EVENT_TYPE;

struct SerializEvent {
    PUBLISH_EVENT_TYPE type;
    gdouble x;
    gdouble y;
    gdouble pressure;
    gdouble xtilt;
    gdouble ytilt;
    unsigned int width;
    unsigned int height;
    int      button;
    int      keyval;
    unsigned int   time;
};

struct PiBoardApp {
	int	                     nn_socket;
        FILE                         *saved;
};

struct PiBoardApp	piboard;

int
main (int    argc,
      char **argv)
{
  memset (&piboard, 0, sizeof (struct PiBoardApp));
  piboard.saved = fopen (".saved.txt", "r");
  if (!piboard.saved)
  {
    fprintf (stderr, "Can't read the saved file.\n");
    return -1;
  }

  piboard.nn_socket = nn_socket (AF_SP, NN_PUB);
  nn_bind (piboard.nn_socket, "tcp://*:7789");

  struct SerializEvent *pe;
  pe = malloc (sizeof (struct SerializEvent) );
  
  unsigned int last = 0;
  unsigned int delay = 0;
  if (fread (pe, sizeof (struct SerializEvent), 1, piboard.saved) == 1)
  {
    delay = (unsigned int)(g_get_monotonic_time() / 1000) - pe->time;
  }
  while (fread (pe, sizeof (struct SerializEvent), 1, piboard.saved) == 1)
  {
    if (last && pe->time - last > 0)
    {
      usleep ((pe->time - last) * 1000);
    }
    last = pe->time;
//    pe->time += delay;
    nn_send (piboard.nn_socket, pe, sizeof (struct SerializEvent), NN_DONTWAIT);
  }
  nn_close(piboard.nn_socket);
  fclose (piboard.saved);
  return 0;
}
