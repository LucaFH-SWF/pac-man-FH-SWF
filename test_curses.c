#ifdef __unix__
#include <unistd.h>
#include <ncurses.h>
#define SLEEP napms(15);
#endif
#ifdef WIN32
#include <windows.h>
#include <ncurses/ncurses.h>
#define SLEEP nsleep(1);
#endif

#include <stdlib.h>
#include <time.h>

void nsleep(long miliseconds){
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = miliseconds;
    nanosleep(&ts, NULL);
}

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
	int speed;
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
	int x;
	int y;
	direction_t direction;
	int speed;
}pacman_t;

void print_board(char **points, xy size, WINDOW*);
char **create_points(xy *size);
char **init_points(xy *size, char *filename);
void pacman_start(pacman_t *pacman);
void print_pacman(pacman_t pacman, WINDOW*);
char richtungtochar(direction_t richtung);
void print_ghosts(ghosts_t ghosts, WINDOW*);
void move_pacman(pacman_t *pacman);
int kollision_richtung(pacman_t pacman, direction_t richtung, char **points);
int kollision_move(pacman_t pacman, char **points);
xy next_move(pacman_t pacman, direction_t direction);//gibt die nächste position von pacman bei angegebener Richtung zurück

int main()
{
	xy size;
	char **points = NULL;
	points = init_points(&size, "map.txt");
	if(!points)
		return -1;

	pacman_t pacman;
	pacman_start(&pacman);
	
	ghosts_t ghosts;
	ghosts.red.x = 5;
	ghosts.red.y = 5;
	ghosts.red.state = chase;
	ghosts.pink.x = 7;
	ghosts.pink.y = 5;
	ghosts.pink.state = chase;
	ghosts.cyan.x = 9;
	ghosts.cyan.y = 5;
	ghosts.cyan.state = chase;
	ghosts.orange.x = 11;
	ghosts.orange.y = 5;
	ghosts.orange.state = chase;

	initscr();
	WINDOW *game = initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	curs_set(0);

	start_color();

	init_pair(1, COLOR_BLUE, COLOR_BLUE);
	init_pair(2, COLOR_BLACK, COLOR_YELLOW);
	init_pair(3, COLOR_BLACK, COLOR_RED);
	init_pair(4, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(5, COLOR_BLACK, COLOR_CYAN);
	init_pair(6, COLOR_BLACK, COLOR_YELLOW);

	init_pair(7, COLOR_WHITE, COLOR_BLACK);
	
	int run = 1;

	int pressed_key = 0;

	int move = 0;

	direction_t input = neutral;

	flushinp();

	while(run) //action loop
	{

		// ===== Benutzereingaben =====
        pressed_key = getch();
		
        switch(pressed_key)
        {
        	case ERR:
        		break;
            case KEY_UP:
                input = up;
            	break;
			case KEY_DOWN:
                input = down;
            	break;
            case KEY_LEFT:
                input = left;
            	break;
            case KEY_RIGHT:
                input = right;
            	break;
            case 'q':
                run = 0;
		}

		flushinp();
		SLEEP
		
		move++;
		//====Kolision + Geister====
		if(move >= pacman.speed) //alle 150ms
		{
			move = 0;
			if(kollision_richtung(pacman, input, points))
			{
				pacman.direction = input;
				move_pacman(&pacman);
			}
			else
			{
				if(kollision_move(pacman, points))
					move_pacman(&pacman);
			}
		
		//bewege Geister

		//kollision geist pacman?
		//kollision -> game over
		//keine kollision -> geist bewegen
		
		//====PRINT====
			erase();

			print_board(points, size, game);

			print_pacman(pacman, game);

			print_ghosts(ghosts, game);

			refresh();
		}
	}

	endwin();
	return 0;
}

char **create_points(xy *size)
{
	char **points;
	
	points = (char **) malloc(size->x * sizeof(points)); //sizey viele pointer auf pointer

	if(!points)
		return NULL;
	
	for(int i = 0; i < size->x; ++i) //sizex viele char reservieren für jeden pointer
	{
		points[i] = (char *) malloc(size->y * sizeof(char));
		if(!points[i])
			return NULL;
	}

	return points;
}

char **init_points(xy *size, char* filename)
{
	FILE *fp;
	fp = fopen(filename, "r");
	if(!fp)
	{
		printf("FEHLER konnte map Datei nicht öffnen!");
		return NULL;
	}
		
	fscanf(fp, "%d", &size->x);
	fscanf(fp, "%d", &size->y);
	fgetc(fp);
	char **points = NULL;
	points = create_points(size);
	if(!points)
	{
		printf("FEHLER konnte Speicher für Spielfeld nicht allokieren!");
		return NULL;
	}


	for(int i = 0; i < size->y; ++i)
	{
		for(int j = 0; j < size->x; ++j)
		{
			points[j][i] = fgetc(fp);
		}
		fgetc(fp);
	}
	fclose(fp);
	return points;
}

void print_board(char **points, xy size, WINDOW* win)
{
	for(int i = 0; i < size.y; ++i)
	{
		for(int j = 0; j < size.x; ++j)
		{
			if(points[j][i] == 'W')
			{
				attron(COLOR_PAIR(1));
					mvwprintw(win, 5+i, 5+j, "%c", points[j][i]);
				attroff(COLOR_PAIR(1));
			}
			else
			{
				attron(COLOR_PAIR(7));
					mvwprintw(win, 5+i, 5+j, "%c", points[j][i]);
				attroff(COLOR_PAIR(7));
			}
		}
	}
}

void print_pacman(pacman_t pacman, WINDOW* win)
{
	attron(COLOR_PAIR(2));
	mvwprintw(win, pacman.y + 5, pacman.x + 5, "%c", richtungtochar(pacman.direction));
	attroff(COLOR_PAIR(2));
}

char richtungtochar(direction_t richtung)
{
	switch(richtung)
	{
		case up:
			return '^';
			break;
		case down:
			return 'v';
			break;
		case left:
			return '<';
			break;
		case right:
			return '>';
			break;
		default:
			return 'P';
			break;
	}
}

void print_ghosts(ghosts_t ghosts, WINDOW* win)
{
	//red
	attron(COLOR_PAIR(3));
	mvwprintw(win, ghosts.red.y + 5, ghosts.red.x + 5, "%c", 'R');
	attroff(COLOR_PAIR(3));
	//pink
	attron(COLOR_PAIR(4));
	mvwprintw(win, ghosts.pink.y + 5, ghosts.pink.x + 5, "%c", 'P');
	attroff(COLOR_PAIR(4));
	//cyan
	attron(COLOR_PAIR(5));
	mvwprintw(win, ghosts.cyan.y + 5, ghosts.cyan.x + 5, "%c", 'C');
	attroff(COLOR_PAIR(5));
	//orange
	attron(COLOR_PAIR(6));
	mvwprintw(win, ghosts.orange.y + 5, ghosts.orange.x + 5, "%c", 'O');
	attroff(COLOR_PAIR(6));		
}

void move_pacman(pacman_t *pacman)
{
	pacman->x = next_move(*pacman, pacman->direction).x;
	pacman->y = next_move(*pacman, pacman->direction).y;
}

int kollision_richtung(pacman_t pacman, direction_t richtung, char **points)
{
	pacman.x = next_move(pacman, richtung).x;
	pacman.y = next_move(pacman, richtung).y;
	if(points[pacman.x][pacman.y] != 'W')
		return 1;
	else
		return 0;
}

int kollision_move(pacman_t pacman, char **points)
{
	pacman.x = next_move(pacman, pacman.direction).x;
	pacman.y = next_move(pacman, pacman.direction).y;
	if(points[pacman.x][pacman.y] != 'W')
		return 1;
	else
		return 0;
}

xy next_move(pacman_t pacman, direction_t direction)
{
	xy pos;
	pos.x = pacman.x;
	pos.y = pacman.y;
	switch(direction)
	{
	case up:
		pos.y -= 1;
		break;
	case down:
		pos.y += 1;
		break;
	case left:
		pos.x -= 1;
		break;
	case right:
		pos.x +=1;
		break;
	default:
		break;
	}	
	return pos;
}

void pacman_start(pacman_t *pacman)
{
	pacman->x = 10;
	pacman->y = 10;
	pacman->speed = 15;
	pacman->direction = neutral;	
}
