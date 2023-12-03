#include <ncurses.h>
#include <stdlib.h>

void print_src(char **walls, int sizex, int sizey);
void print_src2(char **walls, int sizex, int sizey);

int main()
{
	int quit = 1;
	int sizex = 50;
	int sizey = 30;

	char **walls;

	walls = (char **) malloc(sizey * sizeof(walls)); //sizey viele pointer auf pointer

	for(int i = 0; i < sizey; ++i) //sizex viele char reservieren für jeden pointer
		walls[i] = (char *) malloc(sizex * sizeof(char));

	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			walls[i][j] = 'W';
		}
	}


	char **points;

	points = (char **) malloc(sizey * sizeof(points)); //sizey viele pointer auf pointer

	for(int i = 0; i < sizex; ++i) //sizex viele char reservieren für jeden pointer
		points[i] = (char *) malloc(sizex * sizeof(char));

	for(int i = 0; i < sizex; ++i)
	{
		for(int j = 0; j < sizey; ++j)
		{
			points[j][i] = '\0';
		}
	}

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
	print_src(walls, sizex, sizey);

	print_src2(points, sizex, sizey);

	attron(COLOR_PAIR(2));
	mvprintw(10 + 5, 10 + 5, "%c", 'P');
	attroff(COLOR_PAIR(2));

	refresh();

	getchar();
	endwin();
	return 0;
}

void print_src(char **walls, int sizex, int sizey)
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
			mvprintw(5+i, 5+j, "%c", 'W');//walls[i][j]);
		}
	}
}
