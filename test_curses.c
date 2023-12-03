#include <ncurses.h>
#include <stdlib.h>

void print_src(char **walls, int sizex, int sizey);

int main()
{
	int quit = 1;
	int sizex = 10;
	int sizey = 10;
	
	//char **walls = (char **) malloc(sizex*sizey * sizeof(char));
	char walls[sizex][sizey];

	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			walls[i][j] = 'a';
		}
	}
	
	walls[1][2] = 'a';

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	curs_set(0);
/*
	while(!quit)//action-loop
	{
		
	}
*/
	getchar();
	//print_src(walls, sizex, sizey);
	mvprintw(5, 5, "%c", walls[1][2]);
	refresh();
	endwin();
	
	getchar();
	return 0;
}

void print_src(char **walls, int sizex, int sizey)
{
	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			mvprintw(5+i, 5+j, "%c", walls[j][i]);
		}
	}
}
