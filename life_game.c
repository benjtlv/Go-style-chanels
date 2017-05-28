#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "life_game.h"

struct coord coord(short int x, short int y){
  struct coord c;
  c.x = x;
  c.y = y;
  return c;
}

struct coord* set_neighbors(struct cell* cell, int x, int y, int max_x, int max_y){
  struct coord* c;
  if (x == max_x - 1){
    if (y == max_y - 1){
      c = malloc(sizeof(struct coord) * 3);
      c[0]= coord(x-1, y);
      c[1]= coord(x-1, y-1);
      c[2]= coord(x, y-1);
      cell->nb_neighbors = 3;
    }
    else if (y == 0){
      c = malloc(sizeof(struct coord) * 3);
      c[0]= coord(x-1, y);
      c[1]= coord(x-1, y+1);
      c[2]= coord(x, y+1);
      cell->nb_neighbors = 3;
    }
    else{
      c = malloc(sizeof(struct coord) * 5);
      c[0]= coord(x-1, y);
      c[1]= coord(x-1, y-1);
      c[2]= coord(x, y-1);
      c[3]= coord(x-1, y+1);
      c[4]= coord(x, y+1);
      cell->nb_neighbors = 5;
    }
  }
  else if (x == 0){
    if (y == max_y - 1){
      c = malloc(sizeof(struct coord) * 3);
      c[0]= coord(x+1, y);
      c[1]= coord(x+1, y-1);
      c[2]= coord(x, y-1);
      cell->nb_neighbors = 3;
    }
    else if (y == 0){
      c = malloc(sizeof(struct coord) * 3);
      c[0]= coord(x+1, y);
      c[1]= coord(x+1, y+1);
      c[2]= coord(x, y+1);
      cell->nb_neighbors = 3;
    }
    else{
      c = malloc(sizeof(struct coord) * 5);
      c[0]= coord(x+1, y);
      c[1]= coord(x+1, y-1);
      c[2]= coord(x, y-1);
      c[3]= coord(x+1, y+1);
      c[4]= coord(x, y+1);
      cell->nb_neighbors = 5;
    }
  }
  else{
    if (y == max_y - 1){
      c = malloc(sizeof(struct coord) * 5);
      c[0]= coord(x+1, y);
      c[1]= coord(x+1, y-1);
      c[2]= coord(x, y-1);
      c[3]= coord(x-1, y-1);
      c[4]= coord(x-1, y);
      cell->nb_neighbors = 5;
    }
    else if (y == 0){
      c = malloc(sizeof(struct coord) * 5);
      c[0]= coord(x+1, y);
      c[1]= coord(x+1, y+1);
      c[2]= coord(x, y+1);
      c[3]= coord(x-1, y+1);
      c[4]= coord(x-1, y);
      cell->nb_neighbors = 5;
    }
    else{
      c = malloc(sizeof(struct coord) * 8);
      c[0]= coord(x+1, y);
      c[1]= coord(x+1, y+1);
      c[2]= coord(x, y+1);
      c[3]= coord(x-1, y+1);
      c[4]= coord(x-1, y);
      c[5]= coord(x+1, y-1);
      c[6]= coord(x, y-1);
      c[7]= coord(x-1, y-1);
      cell->nb_neighbors = 8;
    }
  }
  return c;
}

struct cell* create_cell(int x, int y, int max_x, int max_y){
  struct cell* c = malloc(sizeof(struct cell));
  int r = rand()%2;
  c->state = r;
  c->neighbors = set_neighbors(c, x, y, max_x, max_y);
  return c;
}
      
struct cell* copy_cell(struct cell* c){
  struct cell* new_c = malloc(sizeof(struct cell));
  memcpy (new_c, c, sizeof(struct cell));
  return new_c;
}

struct world* copy_world(struct world* w){
  struct world* new_w = malloc(sizeof(struct world));
  new_w->x = w->x;
  new_w->y = w->y;
  new_w->world = malloc(sizeof(struct cell**) * w->x);
  for (int i = 0; i<w->x; i++){
    new_w->world[i] = malloc(sizeof(struct cell*) * w->y);
    for (int j = 0; j<w->y; j++){
      new_w->world[i][j] = copy_cell(w->world[i][j]);
    } 
  }
  return new_w;
}

struct world* new_world(int x, int y){
  struct world* w = malloc(sizeof(struct world));
  w->world = malloc(sizeof(struct cell**) * x);
  w->x = x;
  w->y = y;
  for (int i = 0; i<x; i++){
    w->world[i] = malloc(sizeof(struct cell*) * y);
    for (int j = 0; j<y; j++){
      w->world[i][j] = create_cell(i, j, x, y);
    } 
  }
  return w;
}

struct life_game* new_life_game(int x, int y){
  srand(time(NULL));
  struct life_game* lg = malloc(sizeof(struct life_game));
  lg->previous = new_world(x, y);
  lg->current = copy_world(lg->previous);
  return lg;
}

void update_current_world(struct life_game* lg, int x, int y, enum state s){
  lg->current->world[x][y]->state = s;
}

void free_cell(struct cell* c){
  free(c);
}

void free_world(struct world* w){
  for (int i = 0; i<w->x; i++){
    for (int j = 0; j<w->y; j++)
      free_cell(w->world[i][j]);
    free(w->world[i]);
  }
  free(w->world);
  free(w);
}

void next_gen_preparation(struct life_game* lg){
  free_world(lg->previous);
  lg->previous = copy_world(lg->current);
}

int nb_live_neighbors(struct cell* c, struct world* w){
  int len = c->nb_neighbors;
  int n = 0;
  for(int i = 0; i<len; i++){
    struct coord co = c->neighbors[i];
    if(w->world[co.x][co.y]->state == ALIVE)
      n ++;
  }
  return n;
}

enum state compute_state(int x, int y, struct life_game* lg){
  int n = nb_live_neighbors(lg->previous->world[x][y], lg->previous);
  enum state cs = lg->previous->world[x][y]->state;
  if(cs == ALIVE){
    if(n<2)
      return DEAD;
    if(n<4)
      return ALIVE;
    else
      return DEAD;
  }
  else{
    if(n == 3)
      return ALIVE;
    else
      return DEAD;
  }
}
