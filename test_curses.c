#include <ncurses.h>
#include <stdlib.h>

typedef struct{
	int x;
	int y;
} xy;

typedef struct{
	xy *xy;
	char *richtung;
}Tpacman;

void print_background(xy* size);
void print_points(char **points, xy* size);
char **create_points(xy* size);
void init_points(char **points, xy* size);
xy *new_xy(int x, int y);

int main()
{
	xy *size;
	size = new_xy(50 ,30);

	Tpacman *pacman = (Tpacman *) malloc(sizeof(Tpacman));
	pacman->xy = new_xy(10,10);

	char **points = create_points(size);

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	curs_set(0);


	start_color();

	init_pair(1, COLOR_BLUE, COLOR_BLUE);

	init_pair(2, COLOR_BLACK, COLOR_YELLOW);

	print_background(size);

	print_points(points, size);

	attron(COLOR_PAIR(2));
	mvprintw(pacman->xy->y + 5, pacman->xy->x + 5, "%c", 'P');
	attroff(COLOR_PAIR(2));

	refresh();

	getchar();
	endwin();
	return 0;
}

char **create_points(xy* size)
{
	char **points;
	
	points = (char **) malloc(size->x * sizeof(points)); //sizey viele pointer auf pointer
	
	for(int i = 0; i < size->x; ++i) //sizex viele char reservieren für jeden pointer
		points[i] = (char *) malloc(size->y * sizeof(char));

	init_points(points, size);
	return points;
}

xy *new_xy(int x, int y)
{
	xy *new = (xy *) malloc(sizeof(xy));
	new->x = x;
	new->y = y;
	return new;
}

void init_points(char **points, xy* size)
{
	for(int i = 0; i < size->x; ++i)
	{
		for(int j = 0; j < size->y; ++j)
		{
			points[i][j] = '\0';
		}
	}
	points[1][2] = '.';
	points[3][2] = 'o';
	points[5][2] = ' ';

}

void print_background(xy* size)
{
	attron(COLOR_PAIR(1));
	for(int i = 0; i < size->y; ++i)
	{
		for(int j = 0; j < size->x; ++j)
		{
			mvprintw(5+i, 5+j, "%c", ' ');
		}
	}
	attroff(COLOR_PAIR(1));
}

void print_points(char **points, xy* size)
{
	for(int i = 0; i < size->y; ++i)
	{
		for(int j = 0; j < size->x; ++j)
		{
			mvprintw(5+i, 5+j, "%c", points[j][i]);
		}
	}
}
