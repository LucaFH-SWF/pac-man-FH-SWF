int kollision_richtung(pacman_t *pacman, direction_t richtung, char **points);
int kollision_move(pacman_t *pacman, char **points);
int isoob(pacman_t pacman, xy size); //isoob = out of bounds?
int oob(pacman_t *pacman, xy size); //oob = out of bounds handeling

main:

/*			if(!isoob(pacman, size))
			{
				kollision_richtung(&pacman, input, points);
				kollision_move(&pacman, points);
			}
			else
			{
				oob(&pacman, size);
			}
*/

int kollision_richtung(pacman_t *pacman, direction_t richtung, char **points)
{
	int x = next_move(*pacman, richtung).x;
	int y = next_move(*pacman, richtung).y;
	if(points[x][y] == 'W')
		return 0;
	else
	{
		pacman->direction = richtung;
	}
}

int kollision_move(pacman_t *pacman, char **points)
{
		int x = next_move(*pacman, pacman->direction).x;
		int y = next_move(*pacman, pacman->direction).y;
		if(points[x][y] == 'R')
				//reset_pacman(pacman);
		if(points[x][y] == 'W')
			return 0;
		else
			move_pacman(pacman);
}

int isoob(pacman_t pacman, xy size)
{
	int x = next_move(pacman, pacman.direction).x;
	int y = next_move(pacman, pacman.direction).y;

	mvwprintw(stdscr, 8 , 5, "nextx: %2d nexty: %2d", x, y);
	
	if(x>size.x-1)
	{
		return 1;
	}
	if(y>size.y-1)
	{
		return 1;
	}
	if(x<0)
	{
		return 1;
	}
	if(y<0)
	{
		return 1;
	}
	return 0;
}

int oob(pacman_t *pacman, xy size)
{
	int x = next_move(*pacman, pacman->direction).x;
	int y = next_move(*pacman, pacman->direction).y;
	
	if(x>size.x-1)
	{
		pacman->x = 0;
	}
	if(y>size.y-1)
	{
		pacman->y = 0;
	}
	if(x<0)
	{
		pacman->x = size.x-1;
	}
	if(y<0)
	{
		pacman->y = size.y;
	}
	return 1;
}