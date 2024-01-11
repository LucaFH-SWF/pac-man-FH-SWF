#ifndef pacman_types
#define pacman_types

typedef enum {up, right, down, left, neutral} direction_t;
typedef enum {chase, frightened, scatter, idle} state_t;

struct xy{
	int x;
	int y;
};

typedef struct{
	double x;
	double y;
} xy_double;

struct ghost{
	int x;
	int y;
	int start_x;
	int start_y;
	state_t state;
	direction_t direction;
	int speed;
    int traped;
    int frightened_s;
};

typedef struct xy xy;
typedef struct ghost ghost_t;

typedef struct{
	ghost_t red;
	ghost_t pink;
	ghost_t orange;
	ghost_t cyan;
} ghosts_t;

typedef struct{
	int lives;
	int score;
	int dots_collected;
	int dots_tocollect;
	int x;
	int y;
	int start_x;
	int start_y;
	direction_t direction;
	int speed;
}pacman_t;

typedef struct{
	char** field;
	xy size;
	int score;
	xy sp_ghohsts;
	int level;
    int scatter;
    int scatter_n;
}game_t;

#endif