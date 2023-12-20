typedef enum {up, right, down, left, neutral} direction_t;
typedef enum {chase, frightened, scatter} state_t;

struct xy{
	int x;
	int y;
};

struct ghost{
	int x;
	int y;
	state_t state;
	direction_t direction;
};

typedef struct xy xy;
typedef struct ghost ghost_t;

typedef struct{
	ghost_t red;
	ghost_t pink;
	ghost_t orange;
	ghost_t cyan;
	int speed;
} ghosts_t;

typedef struct{
	int x;
	int y;
	direction_t richtung;
	int speed;
}pacman_t;

char **create_points(xy size) //zugriff mit points[x][y]
{
	char **points;
	
	points = (char **) malloc(size.x * sizeof(points)); //sizey viele pointer auf char pointer

	if(!points)
		return NULL;

	for(int i = 0; i < size.x; ++i) //sizex viele char reservieren für jeden pointer
	{
		points[i] = (char *) malloc(size.y * sizeof(char));
		if(!points[i])
			return NULL;
	}

	return points;
}
