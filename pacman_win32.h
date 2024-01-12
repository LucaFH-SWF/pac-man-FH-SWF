#ifndef pacman_win32
#define pacman_win32

#include <sys/timeb.h>

// Gödeker | Funktion um alle 10 ms die Schleife in der main funktion zu durchlaufen ( Windows )

int timing(int *prev_loop) //nimmt einen int pointer an
{
	struct _timeb cur_loop;
	_ftime( &cur_loop );
	//cur_loop.millitm ist der Millisekunden teil der aktuellen Zeit ( also zwischen 0 und 999 ) 

	int val = cur_loop.millitm - *prev_loop; //Differenz der 

	if(val < 0)
		val =( val * (-1) );

	if(val  > 10) //wenn der aktuelle und vorherige durchlauf mehr als 10 ms außeinander liegen
	{
		*prev_loop = cur_loop.millitm;
		return 1;
	}
	else
		return 0;

}

#endif