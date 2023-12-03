#include <ncurses.h>
#include <stdlib.h>

typedef struct{
	int x;
	int y;
} xy;

void print_background(xy* size);
void print_points(char **points, int sizex, int sizey);
char **create_points(int sizex, int sizey);
void init_points(char **points, int sizex, int sizey);
xy *new_xy(int x, int y);

int main()
{
	int quit = 1;
	int sizex = 50;
	int sizey = 30;

	xy *size = new_xy(50 ,30);

	char **points = create_points(sizex, sizey);
	init_points(points, sizex, sizey);

	points[1][2] = '.';
	points[3][2] = 'o';
	points[5][2] = ' ';

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	curs_set(0);

	start_color();

	init_pair(1, COLOR_BLUE, COLOR_BLUE);

	init_pair(2, COLOR_BLACK, COLOR_YELLOW);

/*
	while(!quit)//action-loop
	{
		
	}
*/
	print_background(size);

	print_points(points, sizex, sizey);

	attron(COLOR_PAIR(2));
	mvprintw(10 + 5, 10 + 5, "%c", 'P');
	attroff(COLOR_PAIR(2));

	refresh();

	getchar();
	endwin();
	return 0;
}

char **create_points(int sizex, int sizey)
{
	char **points;
	
	points = (char **) malloc(sizex * sizeof(points)); //sizey viele pointer auf pointer
	
	for(int i = 0; i < sizex; ++i) //sizex viele char reservieren für jeden pointer
		points[i] = (char *) malloc(sizey * sizeof(char));

	return points;
}

void init_points(char **points, int sizex, int sizey)
{
	for(int i = 0; i < sizex; ++i)
	{
		for(int j = 0; j < sizey; ++j)
		{
			points[i][j] = '\0';
		}
	}
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

void print_points(char **points, int sizex, int sizey)
{
	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			mvprintw(5+i, 5+j, "%c", points[j][i]);
		}
	}
}
