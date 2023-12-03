#include <ncurses.h>
#include <stdlib.h>

void print_background(int sizex, int sizey);
void print_src2(char **walls, int sizex, int sizey);
char** init_points(int sizex, int sizey);

int main()
{
	int quit = 1;
	int sizex = 50;
	int sizey = 30;

	char **points = init_points(sizex, sizey);

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
	print_background(sizex, sizey);

	print_src2(points, sizex, sizey);

	attron(COLOR_PAIR(2));
	mvprintw(10 + 5, 10 + 5, "%c", 'P');
	attroff(COLOR_PAIR(2));

	refresh();

	getchar();
	endwin();
	return 0;
}

char** init_points(int sizex, int sizey)
{
	char **points;
	
	points = (char **) malloc(sizex * sizeof(points)); //sizey viele pointer auf pointer
	
	for(int i = 0; i < sizex; ++i) //sizex viele char reservieren für jeden pointer
		points[i] = (char *) malloc(sizey * sizeof(char));
	
	for(int i = 0; i < sizex; ++i)
	{
		for(int j = 0; j < sizey; ++j)
		{
			points[i][j] = '\0';
		}
	}
	return points;
}

void print_background(int sizex, int sizey)
{
	attron(COLOR_PAIR(1));
	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			mvprintw(5+i, 5+j, "%c", ' ');
		}
	}
	attroff(COLOR_PAIR(1));
}

void print_src2(char **walls, int sizex, int sizey)
{
	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			mvprintw(5+i, 5+j, "%c", walls[j][i]);
		}
	}
}
