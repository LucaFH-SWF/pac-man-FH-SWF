#include <ncurses.h>

int main()
{
	int quit = 1;

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	curs_set(0);

	while(!quit)//action-loop
	{
		
	}

	endwin();
	return 0;
}
