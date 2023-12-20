#ifdef __unix__
#include <unistd.h>
#include<ncurses.h>
#define SLEEP napms(12);
#endif
#ifdef WIN32
#include <windows.h>
#include<ncurses/ncurses.h>
#define nsleep(10);
#endif

#include <stdlib.h>
#include <time.h>
#include <errno.h>    

void nsleep(long miliseconds) {
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
	direction_t richtung;
	int speed;
}pacman_t;

void print_background(xy size);
void print_points(char **points, xy size);
char **create_points(xy size);
void init_points(char **points, xy size);
void pacman_start(pacman_t *pacman);
void print_pacman(pacman_t pacman);
char richtungtochar(direction_t richtung);
void print_ghosts(ghosts_t ghosts);
void move_pacman(pacman_t *pacman);
int kollision_richtung(pacman_t pacman, direction_t richtung, char **points);
int kollision_move(pacman_t pacman, char **points);
xy next_move(pacman_t pacman, direction_t direction);//gibt die nächste position von pacman bei angegebener Richtung zurück

int main()
{
	xy size;
	size.x = 50;
	size.y = 30;
	
	char **points = create_points(size);
	if(!points)
		return -1;
	
	init_points(points, size);

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
		SLEEP;

		move++;
		//====Kolision + Geister====
		if(move >= pacman.speed) //alle 150ms
		{
			move = 0;
			if(kollision_richtung(pacman, input, points))
			{
				pacman.richtung = input;
				move_pacman(&pacman);
			}
			else
			{
				if(kollision_move(pacman, points))
					move_pacman(&pacman);
			}
		}
		//bewege Geister

		//kollision geist pacman?
		//kollision -> game over
		//keine kollision -> geist bewegen
		
		//====PRINT====
		erase();

		print_background(size);
		
		print_points(points, size);

		print_pacman(pacman);

		print_ghosts(ghosts);

		refresh();
	}

	endwin();
	return 0;
}

char **create_points(xy size)
{
	char **points;
	
	points = (char **) malloc(size.x * sizeof(points)); //sizey viele pointer auf pointer

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

void init_points(char **points, xy size)
{
	for(int i = 0; i < size.x; ++i)
	{
		for(int j = 0; j < size.y; ++j)
		{
			points[i][j] = '\0';
		}
	}

	for(int i = 2; i < size.x-2; ++i)
	{
		for(int j = 2; j < size.y-2; ++j)
		{
			points[i][j] = ' ';
		}
	}
	points[3][2] = '.';
	points[5][2] = 'o';
	points[7][2] = ' ';

}

void print_background(xy size)
{
	attron(COLOR_PAIR(1));
	for(int i = 0; i < size.y; ++i)
	{
		for(int j = 0; j < size.x; ++j)
		{
			mvprintw(5+i, 5+j, "%c", ' ');
		}
	}
	attroff(COLOR_PAIR(1));
}

void print_points(char **points, xy size)
{
	for(int i = 0; i < size.y; ++i)
	{
		for(int j = 0; j < size.x; ++j)
		{
			mvprintw(5+i, 5+j, "%c", points[j][i]);
		}
	}
}

void print_pacman(pacman_t pacman)
{
	attron(COLOR_PAIR(2));
	mvprintw(pacman.y + 5, pacman.x + 5, "%c", richtungtochar(pacman.richtung));
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

void print_ghosts(ghosts_t ghosts)
{
	//red
	attron(COLOR_PAIR(3));
	mvprintw(ghosts.red.y + 5, ghosts.red.x + 5, "%c", 'R');
	attroff(COLOR_PAIR(3));
	//pink
	attron(COLOR_PAIR(4));
	mvprintw(ghosts.pink.y + 5, ghosts.pink.x + 5, "%c", 'P');
	attroff(COLOR_PAIR(4));
	//cyan
	attron(COLOR_PAIR(5));
	mvprintw(ghosts.cyan.y + 5, ghosts.cyan.x + 5, "%c", 'C');
	attroff(COLOR_PAIR(5));
	//orange
	attron(COLOR_PAIR(6));
	mvprintw(ghosts.orange.y + 5, ghosts.orange.x + 5, "%c", 'O');
	attroff(COLOR_PAIR(6));		
}

void move_pacman(pacman_t *pacman)
{
	pacman->x = next_move(*pacman, pacman->richtung).x;
	pacman->y = next_move(*pacman, pacman->richtung).y;
}

int kollision_richtung(pacman_t pacman, direction_t richtung, char **points)
{
	pacman.x = next_move(pacman, richtung).x;
	pacman.y = next_move(pacman, richtung).y;
	if(points[pacman.x][pacman.y] != '\0')
		return 1;
	else
		return 0;
}

int kollision_move(pacman_t pacman, char **points)
{
	pacman.x = next_move(pacman, pacman.richtung).x;
	pacman.y = next_move(pacman, pacman.richtung).y;
	if(points[pacman.x][pacman.y] != '\0')
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
	pacman->richtung = neutral;	
}
