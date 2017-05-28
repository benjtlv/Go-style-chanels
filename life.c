#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <complex.h>
#include <unistd.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <semaphore.h>
#include "channel.h"
#include "life_game.h"

#define QSIZE 200

struct life_request {
  short int x, y;
};

struct life_reply {
  short int x, y;
  enum state data;
};

struct twochans {
  struct channel *one, *two;
  struct life_game* lg;
};

static enum state
life(short int x, short int y, struct life_game* lg)
{
  return compute_state(x, y, lg);
}

static void *
life_thread(void *arg)
{
  struct twochans chans = *(struct twochans*)arg;
  while(1) {
    struct life_request req;
    struct life_reply rep;
    int rc;

    rc = channel_recv(chans.one, &req);
    if(rc <= 0) {
      channel_close(chans.two);
      return NULL;
    }
    rep.x = req.x;
    rep.y = req.y;
    rep.data = life(req.x, req.y, chans.lg);
    rc = channel_send(chans.two, &rep);
    if(rc < 0) {
      channel_close(chans.two);
      return NULL;
    }
  }
}

static unsigned int
torgb(enum state s)
{
  unsigned char r, g, b;
  if(s == DEAD){
    r = 0;
    g = 0;
    b = 0;
  }
  else{
    r = 255;
    g = 255;
    b = 255;
  }
  return r << 16 | g << 8 | b;
}

static void
paintit(struct twochans *chans, int repcount, cairo_surface_t *surface)
{
  int max_y = chans->lg->current->y;
  unsigned *data = (unsigned*)cairo_image_surface_get_data(surface);
  unsigned rgb;

  for(int i = 0; i < repcount; i++) {
    struct life_reply rep;
    int rc;
    rc = channel_recv(chans->two, &rep);
    if(rc <= 0) {
      perror("channel_recv");
      return;
    }

    int index = rep.x + rep.y * max_y;

    enum state s = rep.data;
    rgb = torgb(s);
    data[index] = rgb;

    update_current_world(chans->lg, rep.x, rep.y, rep.data);
  }
}

sem_t* draw_lock;

gboolean
draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  struct twochans *chans = (struct twochans*)data;
  int max_x = chans->lg->current->x;
  int max_y = chans->lg->current->y;

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, max_x, max_y);


  int repcount = 0;
  int rc;
  struct timespec t0, t1;

  clock_gettime(CLOCK_MONOTONIC, &t0);

  for(int j = 0; j < chans->lg->current->x; j++) {
    for(int i = 0; i < chans->lg->current->y; i ++) {
      struct life_request req;
      req.x = i;
      req.y = j;
      rc = channel_send(chans->one, &req);
      if(rc <= 0) {
	perror("channel_send");
	continue;
      }
      repcount++;
      if(repcount >= QSIZE) {
	paintit(chans, repcount, surface);
	repcount = 0;
      }
    }
  }
  paintit(chans, repcount, surface);

  cairo_surface_mark_dirty (surface);
  cairo_set_source_surface(cr, surface, 0, 0);
  cairo_paint(cr);
  cairo_surface_destroy(surface);

  next_gen_preparation(chans->lg);

  clock_gettime(CLOCK_MONOTONIC, &t1);

  printf("Repaint done in %.2lfs\n",
	 ((double)t1.tv_sec - t0.tv_sec) +
	 ((double)t1.tv_nsec - t0.tv_nsec) / 1.0E9);
  sem_post(draw_lock);
  return FALSE;
}

static void *
refresh_thread(void *arg){
  GtkWidget *widget = (GtkWidget *) arg;
  while(1){
    sem_wait(draw_lock);
    gtk_widget_queue_draw(widget);
  }
  return NULL;
}

int main(int argc, char **argv)
{
  GtkWidget* window;
  GtkWidget* canvas;
  struct twochans chans;
  int numthreads = 0;
  int synchronous = 0;
  int size = 200;
  int rc;
  const char *usage = "./life [-z size] [-n numthreads] [-s]";

  gtk_init(&argc, &argv);

  while(1) {
    int opt = getopt(argc, argv, "z:n:s");
    if(opt < 0)
      break;

    switch(opt) {
    case 'n':
      numthreads = atoi(optarg);
      break;
    case 's':
      synchronous = 1;
      break;
    case 'z':
      size = atoi(optarg);
      break;
    default:
      fprintf(stderr, "%s\n", usage);
      exit(1);
    }
  }

  if(optind < argc) {
    fprintf(stderr, "%s\n", usage);
    exit(1);
  }

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  int s = size;
  if(s<150)
    s = 150;
  else if(s>1024)
    s = 1024;
  gtk_window_set_default_size(GTK_WINDOW(window), s, s);

  canvas = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(window), canvas);

  chans.one = channel_create(sizeof(struct life_request), QSIZE, 0);
  if(chans.one == NULL) {
    perror("channel_create");
    exit(1);
  }

  chans.two =
    channel_create(sizeof(struct life_reply),
		   synchronous ? 0 : QSIZE, 0);
  if(chans.two == NULL) {
    perror("channel_create");
    exit(1);
  }

  chans.lg = new_life_game(size,size);

  if(numthreads <= 0)
    numthreads = sysconf(_SC_NPROCESSORS_ONLN);
  if(numthreads <= 0) {
    perror("sysconf(_SC_NPROCESSORS_ONLN)");
    exit(1);
  }
  printf("Running %d worker threads.\n", numthreads);

  for(int i = 0; i < numthreads; i++) {
    pthread_t t;
    rc = pthread_create(&t, NULL, life_thread, &chans);
    if(rc != 0) {
      errno = rc;
      perror("pthread_create");
      exit(1);
    }
    pthread_detach(t);
  }

  draw_lock = (sem_t*) malloc(sizeof(sem_t));
  sem_init(draw_lock,0,0);

  g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  g_signal_connect(canvas, "draw", G_CALLBACK(draw_callback), &chans);

  gtk_widget_show_all(window);

  pthread_t t;
  rc = pthread_create(&t, NULL, refresh_thread, window);

  gtk_main();

  return 0;
}
