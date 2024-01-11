#ifdef __unix__
#include <unistd.h>
#include <ncurses.h>
#define SLEEP napms(15);
#endif
#ifdef WIN32
#include <windows.h>
#include <ncurses/ncurses.h>
#define SLEEP nsleep(1);
#endif

#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "pac_types.h"

void nsleep(long miliseconds){
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = miliseconds;
    nanosleep(&ts, NULL);
}

char **create_field(xy size);//alokieren des Speichers für das Spielfeld
void init_field(char *filename, pacman_t *pacman, ghosts_t *ghosts, game_t *game);//initialisieren von: Spielfeld, Pacman, Geister
void reset_pacman(pacman_t *pacman, direction_t *input);//Pacman auf Startposition setzen und 1 Leben abziehen
void reset_ghosts(ghosts_t *ghosts);//Geister auf Startposition setzen
void reset_ghost(ghost_t *ghost);//einen Geist auf Startposition setzen
void print_pacman(pacman_t pacman, WINDOW*);//Ausgabe Pacman
void print_ghosts(ghosts_t ghosts, WINDOW*);//Ausgabe Geister
void print_board(char **field, xy size, WINDOW*);//Ausgabe des Spielfeldes
char richtungtochar(direction_t richtung);//Pfeil Pacman
xy next_move(int x, int y, direction_t direction, xy size);//gibt die nächste position bei angegebener Richtung zurück
xy next_move_left(int x, int y, direction_t direction, xy size);//gibt die nächste position links bei angegebener Richtung zurück
xy next_move_right(int x, int y, direction_t direction, xy size);//gibt die nächste position rechts bei angegebener Richtung zurück
void move_pacman(pacman_t *pacman, xy size);//Verschiebt die x un y Werte von Pacman in seine Bewegungs Richtung
void pacman_kollision(pacman_t *pacman, direction_t *input, ghosts_t *ghosts, game_t *game);//Kollisions abfrage Pacman
int pacman_geister_kollision(pacman_t *pacman, ghosts_t *ghosts, int *score);//kollisons abfrage pacman und alle Geister
int pacman_geist_kollision(pacman_t *pacman, ghost_t *ghost, int *score);//kollisons abfrage pacman und ein Geist
void frighten_ghosts(ghosts_t *ghosts);//Geister Richtung umkehren und state ändern
void spawn_ghost(ghost_t *ghost, xy *pos);//Geister auf dem Spielfeld platzieren
void move_ghost_red(ghost_t *red, pacman_t *pacman, char **field, xy size);
void move_ghost_pink(ghost_t *pink, pacman_t *pacman, char **field, xy size);
void move_ghost_orange(ghost_t *orange, pacman_t *pacman, char **field, xy size);
void move_ghost_cyan(ghost_t *cyan, pacman_t *pacman, char **field, xy size);
void path_ghost_to_xy(ghost_t *ghost, int x, int y, char **field, xy size);
direction_t get_next_direction_ghost(int x, int y, direction_t direction, int to_x, int to_y, char **field, xy size);
double get_next_next_move_ghost(int x, int y, direction_t direction,int to_x, int to_y, char **field, xy size);
direction_t direction_left(direction_t direction);
direction_t direction_right(direction_t direction);
void reverse_direction(direction_t *direction);

int absolut(int number)
{
	if(number > 0)
		return number;
	else
		return ( number * (-1) );
}

double distance_xy(int x, int y, int to_x, int to_y)
{
	return sqrt (  pow( absolut(to_x - x), 2 ) + pow( absolut(to_y - y), 2 )  );
}

int main()
{
	game_t game;
	xy size;
	pacman_t pacman;
	ghosts_t ghosts;
	game.field = NULL;
	init_field("map.txt", &pacman, &ghosts, &game);
	if(!game.field)
		return -1;

	initscr();
	cbreak();//strg - c zum beenden des Programms
	noecho();
	curs_set(0);

	int x_max, y_max;
	getmaxyx(stdscr, y_max, x_max);

	WINDOW *game_win = newwin(game.size.y, game.size.x, y_max/2 - game.size.y/2, x_max/2 - game.size.x/2);
	refresh();//window auf stdscr "regestrieren?"
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	start_color();

	init_pair(1, COLOR_BLUE, COLOR_BLUE);
	init_pair(2, COLOR_BLACK, COLOR_YELLOW);
	init_pair(3, COLOR_BLACK, COLOR_RED);
	init_pair(4, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(5, COLOR_BLACK, COLOR_CYAN);
	init_pair(6, COLOR_BLACK, COLOR_YELLOW);

	init_pair(7, COLOR_WHITE, COLOR_BLACK); //spielfeld
	init_pair(8, COLOR_WHITE, COLOR_BLUE);
	init_pair(8, COLOR_BLUE, COLOR_WHITE);
	
	int run = 1;

	int pressed_key = 0;

	int move_pacman = 0;
	int move_red = 0;
	int move_pink = 0;
	int move_orange = 0;
	int move_cyan = 0;

	direction_t input = neutral;

	flushinp();

	while(run) //action loop
	{
		// ===== Benutzereingaben =====
        pressed_key = getch();
		
        switch(pressed_key)
        {
        	case ERR:
        		break;
            case KEY_UP:
                input = up;
            	break;
			case KEY_DOWN:
                input = down;
            	break;
            case KEY_LEFT:
                input = left;
            	break;
            case KEY_RIGHT:
                input = right;
            	break;
            case 'q':
                run = 0;
		}

		flushinp();//bereits vorgemerkte eingaben löschen
		SLEEP

		//===PACMAN===
		move_pacman++;

		if(move_pacman >= pacman.speed)
		{
			move_pacman = 0;
			
			pacman_kollision(&pacman, &input, &ghosts, &game);

			mvwprintw(stdscr, 5, 5, "x: %2d y: %2d", pacman.x, pacman.y);//============TEST-AUSGABE===========
			mvwprintw(stdscr, 6, 5, "lives: %1d", pacman.lives);
			mvwprintw(stdscr, 7, 5, "score: %6d", game.score);
			mvwprintw(stdscr, 8, 5, "dots_collected: %6d", pacman.dots_collected);//============TEST-AUSGABE===========
		}
		
		//===GEISTER===
		move_red++;
		if(move_red >= ghosts.red.speed)
		{
			move_red = 0;
			//bewege Geist
			move_ghost_red(&ghosts.red, &pacman, game.field, game.size);
			//kollision
			if(pacman_geister_kollision(&pacman, &ghosts, &game.score))
			{
				reset_pacman(&pacman, &input);
				reset_ghosts(&ghosts);
			}
		}
		move_pink++;
		if(move_pink >= ghosts.pink.speed)
		{
			move_pink = 0;
			//bewege Geist
			move_ghost_pink(&ghosts.pink, &pacman, game.field, game.size);
			//kollision
			if(pacman_geister_kollision(&pacman, &ghosts, &game.score))
			{
				reset_pacman(&pacman, &input);
				reset_ghosts(&ghosts);
			}
		}
		move_orange++;
		if(move_orange >= ghosts.orange.speed)
		{
			move_orange = 0;
			//bewege Geist
			move_ghost_orange(&ghosts.orange, &pacman, game.field, game.size);
			//kollision
			if(pacman_geister_kollision(&pacman, &ghosts, &game.score))
			{
				reset_pacman(&pacman, &input);
				reset_ghosts(&ghosts);
			}
		}
		move_cyan++;
		if(move_cyan >= ghosts.cyan.speed)
		{
			move_cyan = 0;
			//bewege Geist
			move_ghost_cyan(&ghosts.cyan, &pacman, game.field, game.size);
			//kollision
			if(pacman_geister_kollision(&pacman, &ghosts, &game.score))
			{
				reset_pacman(&pacman, &input);
				reset_ghosts(&ghosts);
			}
		}

		//==Spielverlauf==beeinflussend=====
		spawn_ghost(&ghosts.red, &game.sp_ghohsts);
		
		if(pacman.dots_collected > 30)
		{
			spawn_ghost(&ghosts.pink, &game.sp_ghohsts);
		}

		//spawn_ghost(&ghosts.cyan, &game.sp_ghohsts);
		if(game.level > 0 && pacman.dots_collected > 60)
		{
			spawn_ghost(&ghosts.orange, &game.sp_ghohsts);
		}

		//====PRINT / AUSGABE====
		werase(game_win);

		print_board(game.field, game.size, game_win);

		print_pacman(pacman, game_win);

		print_ghosts(ghosts, game_win);
		wrefresh(game_win);
	
		if(pacman.lives < 0)
		{
			run = 0;
		}
	}

	endwin();
	return 0;
}

char **create_field(xy size)
{
	char **field;
	
	field = (char **) malloc(size.x * sizeof(field)); //sizey viele pointer auf pointer

	if(!field)
		return NULL;
	
	for(int i = 0; i < size.x; ++i) //sizex viele char reservieren für jeden pointer
	{
		field[i] = (char *) malloc(size.y * sizeof(char));
		if(!field[i])
			return NULL;
	}

	return field;
}

void init_field(char* filename, pacman_t *pacman, ghosts_t *ghosts, game_t *game)
{
	FILE *fp;
	fp = fopen(filename, "r");
	if(!fp)
	{
		printf("FEHLER konnte map Datei nicht öffnen!");
		return;
	}
		
	fscanf(fp, "%d", &game->size.x);
	fscanf(fp, "%d", &game->size.y);
	fgetc(fp);

	game->field = create_field(game->size);

	if(!game->field)
	{
		printf("FEHLER konnte Speicher für Spielfeld nicht allokieren!");
		game->field = NULL;
		return;
	}

	pacman->lives = 3;
	pacman->dots_collected = 0;
	pacman->dots_tocollect = 0;
	game->score = 0;
	game->level = 0;


	for(int i = 0; i < game->size.y; ++i)
	{
		for(int j = 0; j < game->size.x; ++j)
		{
			game->field[j][i] = fgetc(fp);
			switch(game->field[j][i])
			{
				case '$':
					pacman->x = j;
					pacman->y = i;
					pacman->start_x = j;
					pacman->start_y = i;
					pacman->speed = 15;
					pacman->direction = neutral;	
					game->field[j][i] = ' ';
					break;
				case 'S':
					game->sp_ghohsts.x = j;
					game->sp_ghohsts.y = i;	
					game->field[j][i] = ' ';
					break;
				case 'R':
					ghosts->red.x = j;
					ghosts->red.y = i;
					ghosts->red.start_x = j;
					ghosts->red.start_y = i;
					ghosts->red.state = idle;			
					ghosts->red.direction = left;
					ghosts->red.speed = 20;
					game->field[j][i] = ' ';
					break;
				case 'P':
					ghosts->pink.x = j;
					ghosts->pink.y = i;
					ghosts->pink.start_x = j;
					ghosts->pink.start_y = i;
					ghosts->pink.direction = left;
					ghosts->pink.state = idle;
					ghosts->pink.speed = 20;
					game->field[j][i] = ' ';
					break;
				case 'C':
					ghosts->cyan.x = j;
					ghosts->cyan.y = i;
					ghosts->cyan.start_x = j;
					ghosts->cyan.start_y = i;
					ghosts->cyan.direction = left;
					ghosts->cyan.state = idle;
					ghosts->cyan.speed = 20;
					game->field[j][i] = ' ';
					break;
				case 'O':
					ghosts->orange.x = j;
					ghosts->orange.y = i;
					ghosts->orange.start_x = j;
					ghosts->orange.start_y = i;
					ghosts->orange.direction = left;
					ghosts->orange.state = idle;
					ghosts->orange.speed = 20;
					game->field[j][i] = ' ';
					break;
				case '.':
					pacman->dots_tocollect += 1;
					break;
				case 'o':
					pacman->dots_tocollect += 1;
					break;
			} 
		}
		fgetc(fp);
	}
	fclose(fp);
	return;
}

void print_board(char **field, xy size, WINDOW* win)
{
	for(int i = 0; i < size.y; ++i)
	{
		for(int j = 0; j < size.x; ++j)
		{
			if(field[j][i] == 'W')
			{
				wattron(win, COLOR_PAIR(1));
					mvwprintw(win, i, j, "%c", field[j][i]);
				wattroff(win, COLOR_PAIR(1));
			}
			else
			{
				wattron(win, COLOR_PAIR(7));
					mvwprintw(win, i, j, "%c", field[j][i]);
				wattroff(win, COLOR_PAIR(7));
			}
		}
	}
}

void print_pacman(pacman_t pacman, WINDOW* win)
{
	wattron(win, COLOR_PAIR(2));
	mvwprintw(win, pacman.y, pacman.x, "%c", richtungtochar(pacman.direction));
	wattroff(win, COLOR_PAIR(2));
}

char richtungtochar(direction_t richtung)
{
	switch(richtung)
	{
		case up:
			return '^';
			break;
		case down:
			return 'v';
			break;
		case left:
			return '<';
			break;
		case right:
			return '>';
			break;
		default:
			return 'P';
			break;
	}
}

void print_ghosts(ghosts_t ghosts, WINDOW* win)
{
	//red
	if(ghosts.red.state != frightened)
	{
		wattron(win, COLOR_PAIR(3));
		mvwprintw(win, ghosts.red.y, ghosts.red.x, "%c", 'R');
		wattroff(win, COLOR_PAIR(3));
	}
	else
	{
		if(time(0) % 2)
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.red.y, ghosts.red.x, "%c", 'R');
			wattroff(win, COLOR_PAIR(8));
		}
		else
		{
			wattron(win, COLOR_PAIR(9));
			mvwprintw(win, ghosts.red.y, ghosts.red.x, "%c", 'R');
			wattroff(win, COLOR_PAIR(9));
		}
	}
	//pink
	if(ghosts.pink.state != frightened)
	{
		wattron(win, COLOR_PAIR(4));
		mvwprintw(win, ghosts.pink.y, ghosts.pink.x, "%c", 'P');
		wattroff(win, COLOR_PAIR(4));
	}
	else
	{
		if(time(0) % 2)
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.pink.y, ghosts.pink.x, "%c", 'P');
			wattroff(win, COLOR_PAIR(8));
		}
		else
		{
			wattron(win, COLOR_PAIR(9));
			mvwprintw(win, ghosts.pink.y, ghosts.pink.x, "%c", 'P');
			wattroff(win, COLOR_PAIR(9));
		}
	}
	//cyan
	if(ghosts.cyan.state != frightened)
	{
		wattron(win, COLOR_PAIR(5));
		mvwprintw(win, ghosts.cyan.y, ghosts.cyan.x, "%c", 'C');
		wattroff(win, COLOR_PAIR(5));
	}
	else
	{
		if(time(0) % 2)
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.cyan.y, ghosts.cyan.x, "%c", 'C');
			wattroff(win, COLOR_PAIR(8));
		}
		else
		{
			wattron(win, COLOR_PAIR(9));
			mvwprintw(win, ghosts.cyan.y, ghosts.cyan.x, "%c", 'C');
			wattroff(win, COLOR_PAIR(9));
		}
	}
	//orange
	if(ghosts.orange.state != frightened)
	{
		wattron(win, COLOR_PAIR(6));
		mvwprintw(win, ghosts.orange.y, ghosts.orange.x, "%c", 'O');
		wattroff(win, COLOR_PAIR(6));
	}
	else
	{
		if(time(0) % 2)
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.orange.y, ghosts.orange.x, "%c", 'O');
			wattroff(win, COLOR_PAIR(8));
		}
		else
		{
			wattron(win, COLOR_PAIR(9));
			mvwprintw(win, ghosts.orange.y, ghosts.orange.x, "%c", 'O');
			wattroff(win, COLOR_PAIR(9));
		}
	}
}

void reset_pacman(pacman_t *pacman, direction_t *input)
{
	pacman->x = pacman->start_x;
	pacman->y = pacman->start_y;
	pacman->direction = neutral;
	pacman->lives -= 1;
	*input = neutral;
}

void reset_ghosts(ghosts_t *ghosts)
{
	ghosts->red.x = ghosts->red.start_x;
	ghosts->red.y = ghosts->red.start_y;
	ghosts->red.state = chase;
	ghosts->red.speed = 20;

	ghosts->pink.x = ghosts->pink.start_x;
	ghosts->pink.y = ghosts->pink.start_y;
	ghosts->pink.state = idle;
	ghosts->pink.speed = 20;

	ghosts->orange.x = ghosts->orange.start_x;
	ghosts->orange.y = ghosts->orange.start_y;
	ghosts->orange.state = idle;
	ghosts->orange.speed = 20;

	ghosts->cyan.x = ghosts->cyan.start_x;
	ghosts->cyan.y = ghosts->cyan.start_y;
	ghosts->cyan.state = idle;
	ghosts->cyan.speed = 20;
}

void move_pacman(pacman_t *pacman, xy size)
{
	pacman->x = next_move(pacman->x, pacman->y, pacman->direction, size).x;
	pacman->y = next_move(pacman->x, pacman->y, pacman->direction, size).y;
}

xy next_move(int x, int y, direction_t direction, xy size)
{
	xy pos;
	pos.x = x;
	pos.y = y;
	switch(direction)
	{
		case up:
			pos.y -= 1;
			break;
		case down:
			pos.y += 1;
			break;
		case left:
			pos.x -= 1;
			break;
		case right:
			pos.x +=1;
			break;
		default:
			break;
	}
	//außerhalb des Bereichs
	if(pos.x>size.x-1)
	{
		pos.x = 0;
		return pos;
	}
	if(pos.y>size.y-1)
	{
		pos.y = 0;
		return pos;
	}
	if(pos.x<0)
	{
		pos.x = size.x-1;
		return pos;
	}
	if(pos.y<0)
	{
		pos.y = size.y;
		return pos;
	}
	return pos;
}

xy next_move_left(int x, int y, direction_t direction, xy size)
{
	switch(direction)
	{
		case up:
			return next_move(x, y,direction_left(up), size);
			break;
		case down:
			return next_move(x, y, direction_left(down), size);
			break;
		case left:
			return next_move(x, y, direction_left(left), size);
			break;
		case right:
			return next_move(x, y, direction_left(right), size);
			break;
		default:
			return next_move(x,y,direction, size);
			break;
		
	}
}
xy next_move_right(int x, int y, direction_t direction, xy size)
{
	switch(direction)
	{
		case up:
			return next_move(x, y,direction_right(up), size);
			break;
		case down:
			return next_move(x, y, direction_right(down), size);
			break;
		case left:
			return next_move(x, y, direction_right(left), size);
			break;
		case right:
			return next_move(x, y, direction_right(right), size);
			break;
		default:
			return next_move(x,y,direction,size);
			break;
		
	}
}

direction_t direction_left(direction_t direction)
{
	switch(direction)
	{
		case up:
			return left;
			break;
		case down:
			return right;
			break;
		case left:
			return down;
			break;
		case right:
			return up;
			break;
		default:
			return direction;
			break;
		
	}
}

direction_t direction_right(direction_t direction)
{
	switch(direction)
	{
		case up:
			return right;
			break;
		case down:
			return left;
			break;
		case left:
			return up;
			break;
		case right:
			return down;
			break;
		default:
			return direction;
			break;
		
	}
}

void pacman_kollision(pacman_t *pacman, direction_t *input, ghosts_t *ghosts, game_t *game)
{
	int x = next_move(pacman->x, pacman->y, *input, game->size).x;
	int y = next_move(pacman->x, pacman->y, *input, game->size).y;

	//input übernehmen?
	if(game->field[ next_move(pacman->x, pacman->y, *input, game->size).x ][ next_move(pacman->x, pacman->y, *input, game->size).y ] != 'W')
	{
		pacman->direction = *input;
	}
	else
	{
		x = next_move(pacman->x, pacman->y, pacman->direction, game->size).x;
		y = next_move(pacman->x, pacman->y, pacman->direction, game->size).y;

		if(game->field[x][y] == 'W')
			return;
	}
	
	//Punkte essen
	if(game->field[x][y] == '.')
	{
		game->score += 100;
		pacman->dots_collected += 1;
		game->field[x][y] = ' ';
	}
	//enegizer Punkte essen
	if(game->field[x][y] == 'o')
	{
		game->score += 250;
		pacman->dots_collected += 1;
		game->field[x][y] = ' ';

		frighten_ghosts(ghosts);

		//geschwindigkeit pacman anheben? 
	}
	//erst bewegen, dann kollision mit Geister bestimmen
	move_pacman(pacman, game->size);

	//geister kollision
	if(pacman_geister_kollision(pacman, ghosts, &game->score))
	{
		reset_pacman(pacman, input);
		reset_ghosts(ghosts);
	}	
}

int pacman_geister_kollision(pacman_t *pacman, ghosts_t *ghosts, int *score)
{
	if(pacman_geist_kollision(pacman, &ghosts->red, score))
		return 1;
	if(pacman_geist_kollision(pacman, &ghosts->pink, score))
		return 1;
	if(pacman_geist_kollision(pacman, &ghosts->orange, score))
		return 1;
	if(pacman_geist_kollision(pacman, &ghosts->cyan, score))
		return 1;

	return 0;
}

int pacman_geist_kollision(pacman_t *pacman, ghost_t *ghost, int *score)
{
	if( (ghost->x == pacman->x) && (ghost->y == pacman->y) )
	{
		if(ghost->state != frightened)
		{
			*score = *score + 200;
			return 1;
		}
		else
			reset_ghost(ghost);
	}
	
	return 0;
}

void move_ghost_red(ghost_t *red, pacman_t *pacman, char **field, xy size)
{
	switch(red->state)
	{
		case chase:
			path_ghost_to_xy(red, pacman->x, pacman->y, field, size);
			break;
		case frightened:
			path_ghost_to_xy(red, rand() % size.x, rand() % size.y, field, size);
			break;
		case scatter:
			path_ghost_to_xy(red, 0, 0, field, size);
			break;
		case idle:
			return;
			break;
	}
}

void move_ghost_pink(ghost_t *pink, pacman_t *pacman, char **field, xy size)
{
	switch(pink->state)
	{
		case chase:
			path_ghost_to_xy(pink, pacman->x, pacman->y, field, size);
			break;
		case frightened:
			path_ghost_to_xy(pink, rand() % size.x, rand() % size.y, field, size);
			break;
		case scatter:
			path_ghost_to_xy(pink, size.x, 0, field, size);
			break;
		case idle:
			return;
			break;
	}
}

void move_ghost_orange(ghost_t *orange, pacman_t *pacman, char **field, xy size)
{
	switch(orange->state)
	{
		case chase:
			if( distance_xy(orange->x, orange->y, pacman->x, pacman->y) > 8)
				path_ghost_to_xy(orange, pacman->x, pacman->y, field, size);
			else
				path_ghost_to_xy(orange, 0, size.y, field, size);
			break;
		case frightened:
			path_ghost_to_xy(orange, rand() % size.x, rand() % size.y, field, size);
			break;
		case scatter:
			path_ghost_to_xy(orange, 0, size.y, field, size);
			break;
		case idle:
			return;
			break;
	}
}

void move_ghost_cyan(ghost_t *cyan, pacman_t *pacman, char **field, xy size)
{
	switch(cyan->state)
	{
		case chase:
			path_ghost_to_xy(cyan, pacman->x, pacman->y, field, size);
			break;
		case frightened:
			path_ghost_to_xy(cyan, rand() % size.x, rand() % size.y, field, size);
			break;
		case scatter:
			path_ghost_to_xy(cyan, size.x, size.y, field, size);
			break;
		case idle:
			return;
			break;
	}
}

direction_t get_next_direction_ghost(int x, int y, direction_t direction, int to_x, int to_y, char **field, xy size)
{
	double distance_move = 0.0;
	double distance_left = 0.0;
	double distance_right = 0.0;
	xy move = next_move(x, y, direction, size);
	xy left = next_move_left(x,y, direction, size);
	xy right = next_move_right(x, y, direction, size);
	xy move_2;
	xy left_2;
	xy right_2;

	if(field[move.x][move.y] != 'W')
	{
		distance_move = get_next_next_move_ghost(move.x, move.y, direction, to_x, to_y, field, size);
	}
	else
		distance_move = 1000;

	if(field[left.x][left.y] != 'W')
	{
		distance_left = get_next_next_move_ghost(left.x, left.y, direction_left(direction), to_x, to_y, field, size);
	}
	else
		distance_left = 1000;

	if(field[right.x][right.y] != 'W')
	{
		distance_right = get_next_next_move_ghost(right.x, right.y, direction_right(direction), to_x, to_y, field, size);
	}
	else
		distance_right = 1000;

	if( distance_move < distance_left && distance_move < distance_right )
	{
		//gerade aus
		return direction;
	}
	
	if( distance_left < distance_move && distance_left<distance_right )
	{
		//links
		return direction_left(direction);
	}

	if( distance_right < distance_move && distance_right < distance_left )
	{
		//rechts
		return direction_right(direction);
	}


	//konnte keine Richtung bestimmen
	if( distance_move != 1000 )
	{
		//gerade aus
		return direction;
	}
	
	if( distance_left != 1000 )
	{
		//links
		return direction_left(direction);
	}

	if( distance_right != 1000 )
	{
		//rechts
		return direction_right(direction);
	}


}

double get_next_next_move_ghost(int x, int y, direction_t direction,int to_x, int to_y, char **field, xy size)
{
	double distance_move = 0.0;
	double distance_left = 0.0;
	double distance_right = 0.0;
	xy move = next_move(x, y, direction, size);
	xy left = next_move_left(x,y, direction, size);
	xy right = next_move_right(x, y, direction, size);
	
	//=====Distanz======================================
	if(field[move.x][move.y] != 'W')
		distance_move = distance_xy(move.x, move.y, to_x, to_y);
	else
		distance_move = 1000;

	if(field[left.x][left.y] != 'W')
		distance_left =  distance_xy(left.x, left.y, to_x, to_y);
	else
		distance_left = 1000;

	if(field[right.x][right.y] != 'W')
		distance_right =  distance_xy(right.x, right.y, to_x, to_y);
	else
		distance_right = 1000;
	//==================================================

	//===kleinste Distanz zurück geben==================
	if( distance_move < distance_left && distance_move < distance_right )
	{
		//gerade aus
		return distance_move;
	}
	if( distance_left < distance_move && distance_left<distance_right )
	{
		//links
		return distance_left;
	}

	if( distance_right < distance_move && distance_right < distance_left )
	{
		//rechts
		return distance_right;
	}
	//==================================================
}

void path_ghost_to_xy(ghost_t *ghost, int x, int y, char **field, xy size)
{
	direction_t new_diretion = get_next_direction_ghost(ghost->x, ghost->y, ghost->direction, x, y, field, size);
	ghost->x = next_move(ghost->x, ghost->y, new_diretion, size).x;
	ghost->y = next_move(ghost->x, ghost->y, new_diretion, size).y;
	ghost->direction = new_diretion;
}

void reverse_direction(direction_t *direction)
{
	switch(*direction)
	{
		case up:
			*direction = down;
			break;
		case down:
			*direction = up;
			break;
		case left:
			*direction = right;
			break;
		case right:
			*direction = left;
			break;
		default:
			*direction = neutral;;
			break;
		
	}
}

void reset_ghost(ghost_t *ghost)
{
	ghost->x = ghost->start_x;
	ghost->y = ghost->start_y;
	if(ghost->state == frightened)
		ghost->speed = ghost->speed / 2;
	else
		ghost->speed = 20;
	ghost->state = idle;
}

void frighten_ghosts(ghosts_t *ghosts)
{
	reverse_direction(&ghosts->red.direction);
	reverse_direction(&ghosts->pink.direction);
	reverse_direction(&ghosts->orange.direction);
	reverse_direction(&ghosts->cyan.direction);
	if(ghosts->red.state != idle)
	{
		ghosts->red.state = frightened;
		ghosts->red.speed = ghosts->red.speed * 2;
	}
	if(ghosts->pink.state != idle)
	{
		ghosts->pink.state = frightened;
		ghosts->pink.speed = ghosts->pink.speed * 2;
	}
	if(ghosts->orange.state != idle)
	{
		ghosts->orange.state = frightened;
		ghosts->orange.speed = ghosts->orange.speed * 2;
	}
	if(ghosts->cyan.state != idle)
	{
		ghosts->cyan.state = frightened;
		ghosts->cyan.speed = ghosts->cyan.speed * 2;
	}
}

void spawn_ghost(ghost_t *ghost, xy *pos)
{
	if(ghost->state == idle)
	{
		ghost->x = pos->x;
		ghost->y = pos->y;
		ghost->state = chase;
	}
	else
		return;
}