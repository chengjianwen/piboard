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

#define MOTION_LENGTH	1024
struct tag {
    int tag;
};

struct stroke {
    struct tag tag;
    int   length;
    int   width;
    int   height;
    struct motion {
        double x;
        double y;
        double pressure;
        double xtilt;
        double ytilt;
        unsigned int time;
    } motions[MOTION_LENGTH];
};

struct  key {
    struct tag tag;
    int keyval;
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

  struct stroke stroke;
  stroke.tag.tag = 0x00;
  struct key key;
  key.tag.tag = 0x01;

  unsigned int last = 0;
  unsigned int time;
  char  buf[1024];
  memset (buf, 0, 1024);
  while (fgets(buf, 1024, piboard.saved) != NULL)
  {
    switch (buf[0])
    {
      case 'R':
        sscanf (buf + 2, "%d %d %u", &stroke.width, &stroke.height, &time);
        nn_send (piboard.nn_socket, &stroke, sizeof (struct stroke) - sizeof (struct motion) * (MOTION_LENGTH - stroke.length), NN_DONTWAIT);
       stroke.length = 0;
        break;
      case 'K':
        sscanf (buf + 2, "%d %u", &key.keyval, &time);
        nn_send (piboard.nn_socket, &key, sizeof (struct key), NN_DONTWAIT);
        break;
      case 'M':
        sscanf (buf + 2, 
                "%lf %lf %lf %lf %lf %u",
                &stroke.motions[stroke.length].x,
                &stroke.motions[stroke.length].y,
                &stroke.motions[stroke.length].pressure,
                &stroke.motions[stroke.length].xtilt,
                &stroke.motions[stroke.length].ytilt,
                &time);
          stroke.length = stroke.length + 1;
        break;
      case 'N':
        sscanf (buf + 2, "%u", &time);
        break;
     default:
        break;
    }
    memset (buf, 0, 1024);
    if (last && time - last > 0)
    {
      usleep ((time - last) * 1000);
    }
    last = time;
  }
  nn_close(piboard.nn_socket);
  fclose (piboard.saved);
  return 0;
}
