
enum state {
  ALIVE,
  DEAD
};

struct coord{
  short int x, y;
};

struct cell {
  enum state state;
  short int nb_neighbors;
  struct coord* neighbors;
};

struct world {
  short int x, y;
  struct cell*** world;
};

struct life_game{
  struct world* previous;
  struct world* current;
};

struct life_game* new_life_game(int, int);
void update_current_world(struct life_game*, int, int, enum state);
void next_gen_preparation(struct life_game*);
enum state compute_state(int, int, struct life_game*);
