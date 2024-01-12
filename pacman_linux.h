#ifndef pacman_linux
#define pacman_linux

#include <sys/timeb.h>

// Gödeker | Funktion um alle 10 ms die Schleife in der main funktion zu durchlaufen ( Linux )
// Kommentare siehe "pacman_win32.h"
int timing(int *prev_loop)
{
	struct timeb cur_loop;

	ftime( &cur_loop );

	int val = cur_loop.millitm - *prev_loop;

	if(val < 0)
		val = ( val * (-1) );

	if(val  > 10)
	{
		*prev_loop = cur_loop.millitm;
		return 1;
	}
	else
		return 0;
}


#endif