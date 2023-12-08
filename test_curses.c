	#include <ncurses.h>
#include <stdlib.h>

typedef enum {up, right, down, left} richtung_t;

struct xy{
	int x;
	int y;
};

typedef struct xy xy;
typedef struct xy ghost_t;

typedef struct{
	ghost_t red;
	ghost_t pink;
	ghost_t orange;
	ghost_t cyan;
} ghosts_t;

typedef struct{
	int x;
	int y;
	richtung_t richtung;
}pacman_t;

void print_background(xy size);
void print_points(char **points, xy size);
char **create_points(xy size);
void init_points(char **points, xy size);
void print_pacman(pacman_t pacman);
char richtungtochar(richtung_t richtung);
void print_ghosts(ghosts_t ghosts);
void move_pacman(pacman_t *pacman);

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
	pacman.x = 10;
	pacman.y = 10;

	ghosts_t ghosts;
	ghosts.red.x = 5;
	ghosts.red.y = 5;
	ghosts.pink.x = 7;
	ghosts.pink.y = 5;
	ghosts.cyan.x = 9;
	ghosts.cyan.y = 5;
	ghosts.orange.x = 11;
	ghosts.orange.y = 5;

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

	int pressed_key;

	while(run) //action loop
	{

		// ===== Benutzereingaben =====
        pressed_key = getch(); // Eingabe einlesen
		//flushinp();
        switch(pressed_key)
        {
            case ERR:
                //napms(100); // Pause in Millisekunden
                break;
            case KEY_UP:
                pacman.richtung = up;
                break;
			case KEY_DOWN:
                pacman.richtung = down;
                break;
            case KEY_LEFT:
                pacman.richtung = left;
                break;
            case KEY_RIGHT:
                pacman.richtung = right;
                break;
            case 'q':
                run = 0;
                break;
		}

		napms(100);
		flushinp();

		//tcflush(STDIN_FILENO, TCIFLUSH);
				
		//==Kolision + Geister==
		//kolision pacman u. Wand, pacman und Geister?
		//keine kollision m. Wand -> bewege pacman, Kollision Geist -> Game Over
		move_pacman(&pacman);
		//bewege Geister
		//kollision geist pacman?
		//kollision -> game over

		//==PRINT==
		erase();

		print_background(size);
		print_points(points, size);

		//print pacman
		print_pacman(pacman);

		//print geister
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
	points[1][2] = '.';
	points[3][2] = 'o';
	points[5][2] = ' ';

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

char richtungtochar(richtung_t richtung)
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
	switch(pacman->richtung)
	{
		case up:
			pacman->y -= 1;
		break;
		case down:
			pacman->y += 1;
		break;
		case left:
			pacman->x -= 1;
		break;
		case right:
			pacman->x +=1;
		break;
	}
}
