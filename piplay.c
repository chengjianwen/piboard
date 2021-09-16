/*
 *   程序名：piplay.c
 *     版本：1.0
 *     作者：成建文 <chengjianwen@gmail.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

#pragma pack(4)

typedef enum {
    MOTION,
    PRESSED,
    RELEASED,
    KEY,
    NONE
} PUBLISH_EVENT_TYPE;

struct SerializEvent {
    PUBLISH_EVENT_TYPE type;
    union {
        struct {
            double x;
            double y;
            double pressure;
            double xtilt;
            double ytilt;
        } motion;
        struct {
            int width;
            int height;
        } pressed;
        struct {
            int keyval;
        } key;
    };
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
  char  buf[1024];
  memset (buf, 0, 1024);
  while (fgets(buf, 1024, piboard.saved) != NULL)
  {
    switch (buf[0])
    {
      case 'P':
        pe->type = PRESSED;
        sscanf (buf + 2, "%d %d %u", &pe->pressed.width, &pe->pressed.height, &pe->time);
        break;
      case 'R':
        pe->type = RELEASED;
        sscanf (buf + 2, "%u", &pe->time);
        break;
      case 'K':
        pe->type = KEY;
        sscanf (buf + 2, "%u %u", &pe->key.keyval, &pe->time);
        break;
      case 'M':
        pe->type = MOTION;
        sscanf (buf + 2, 
                "%lf %lf %lf %lf %lf %u",
                &pe->motion.x,
                &pe->motion.y,
                &pe->motion.pressure,
                &pe->motion.xtilt,
                &pe->motion.ytilt,
                &pe->time);
        break;
      case 'N':
        pe->type = NONE;
        sscanf (buf + 2, "%u", &pe->time);
        break;
     default:
        break;
    }
    memset (buf, 0, 1024);
    if (last && pe->time - last > 0)
    {
      usleep ((pe->time - last) * 1000);
    }
    last = pe->time;
    nn_send (piboard.nn_socket, pe, sizeof (struct SerializEvent), NN_DONTWAIT);
  }
  nn_close(piboard.nn_socket);
  fclose (piboard.saved);
  return 0;
}
