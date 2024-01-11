#ifndef pacman_win32
#define pacman_win32
#include <sys/timeb.h>

int timing(int *prev_loop)
{
	struct _timeb cur_loop;
	_ftime( &cur_loop );

	int val = cur_loop.millitm - *prev_loop;

	if(val < 0)
		val =( val * (-1) );

	if(val  > 10)
	{
		*prev_loop = cur_loop.millitm;
		return 1;
	}
	else
		return 0;

}

#endif