#include <ncurses.h>
#include <stdlib.h>

void print_src(char **walls, int sizex, int sizey);

int main()
{
	int quit = 1;
	int sizex = 1000;
	int sizey = 20;
	
	char **walls;

	walls = (char **) malloc(sizey * sizeof(walls)); //sizey viele pointer auf pointer

	for(int i = 0; i < sizey; ++i) //sizex viele char reservieren für jeden pointer
		walls[i] = (char *) malloc(sizex * sizeof(char));

	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			walls[i][j] = 'a';
		}
	}
	
	//walls[1][2] = 'a';

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
	print_src(walls, sizex, sizey);
	//mvprintw(5, 5, "%c", walls[1][2]);
	refresh();
	
	getchar();
	endwin();
	return 0;
}

void print_src(char **walls, int sizex, int sizey)
{
	for(int i = 0; i < sizey; ++i)
	{
		for(int j = 0; j < sizex; ++j)
		{
			mvprintw(5+i, 5+j, "%c", walls[i][j]);
		}
	}
}
