#include <ncurses.h>
#include <stdlib.h>

typedef enum {up, right, down, left} richtung_t;//Uhrzeigersin

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
	
	int quit = 0;

	int pressed_key;
	
	while(!quit) //action loop
	{
		break;
		// ===== Benutzereingaben =====
		
        pressed_key = getch(); // Eingabe einlesen
		
        switch(pressed_key)
        {
            case ERR:
                napms(10); // Pause in Millisekunden
                break;
            case KEY_UP:
                //ausgabe = '^';
                break;
			case KEY_DOWN:
                //ausgabe = 'v';
                break;
            case KEY_LEFT:
                //ausgabe = '<';
                break;
            case KEY_RIGHT:
                //ausgabe = '>';
                break;
            case 'q':
                quit = 1;
                break;
		}
		//==Kolision + Geister==
		//kolision pacman u. Wand, pacman und Geister?
		//keine kollision m. Wand -> bewege pacman, Kollision Geist -> Game Over
		//bewege Geister
		//kollision geist pacman?
		//kollision -> game over

		//==PRINT==
		print_background(size);
		print_points(points, size);
		//print pacman
		attron(COLOR_PAIR(2));
		mvprintw(pacman.y + 5, pacman.x + 5, "%c", 'P');
		attroff(COLOR_PAIR(2));
		//print geister
		attron(COLOR_PAIR(3));
		mvprintw(ghosts.red.y + 5, ghosts.red.x + 5, "%c", 'R');
		attroff(COLOR_PAIR(3));

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
