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

void nsleep(long miliseconds){
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = miliseconds;
    nanosleep(&ts, NULL);
}

typedef enum {up, right, down, left, neutral} direction_t;
typedef enum {chase, frightened, scatter, idle} state_t;

struct xy{
	int x;
	int y;
};

struct ghost{
	int x;
	int y;
	int start_x;
	int start_y;
	state_t state;
	direction_t direction;
	int speed;
};

typedef struct xy xy;
typedef struct ghost ghost_t;

typedef struct{
	ghost_t red;
	ghost_t pink;
	ghost_t orange;
	ghost_t cyan;
} ghosts_t;

typedef struct{
	int lives;
	int score;
	int points_collected;
	int x;
	int y;
	int start_x;
	int start_y;
	direction_t direction;
	int speed;
}pacman_t;

char **create_points(xy *size);//alokieren des Speichers für das Spielfeld
char **init_points(xy *size, char *filename, pacman_t *pacman, ghosts_t *ghosts);//initialisieren von: Spielfeld, Pacman, Geister
void reset_pacman(pacman_t *pacman, direction_t *input);//Pacman auf Startposition setzen und 1 Leben abziehen
void reset_ghosts(ghosts_t *ghosts);//Geister auf Startposition setzen
void print_pacman(pacman_t pacman, WINDOW*);//Ausgabe Pacman
void print_ghosts(ghosts_t ghosts, WINDOW*);//Ausgabe Geister
void print_board(char **points, xy size, WINDOW*);//Ausgabe des Spielfeldes
char richtungtochar(direction_t richtung);//Pfeil Pacman
xy next_move(int x, int y, direction_t direction, xy size);//gibt die nächste position bei angegebener Richtung zurück
xy next_move_left(int x, int y, direction_t direction, xy size);//gibt die nächste position links bei angegebener Richtung zurück
xy next_move_right(int x, int y, direction_t direction, xy size);//gibt die nächste position rechts bei angegebener Richtung zurück
void move_pacman(pacman_t *pacman, xy size);//Verschiebt die x un y Werte von Pacman in seine Bewegungs Richtung
void pacman_kollision(pacman_t *pacman, direction_t *input, char **points, xy size, ghosts_t *ghosts);//Kollisions abfrage Pacman
int pacman_geister_kollision(pacman_t *pacman, ghosts_t *ghosts);//kollisons abfrage pacman und alle Geister
int pacman_geist_kollision(pacman_t *pacman, ghost_t ghost);//kollisons abfrage pacman und ein Geist
void move_ghost_red(ghost_t *red, pacman_t *pacman, char **points, xy size);
direction_t get_next_move_ghost(int x, int y, direction_t direction,pacman_t *pacman, char **points, xy size);
double get_next_next_move_ghost(int x, int y, direction_t direction,pacman_t *pacman, char **points, xy size);
direction_t direction_left(direction_t direction);
direction_t direction_right(direction_t direction);

int absolut(int number)
{
	if(number > 0)
		return number;
	else
		return ( number * (-1) );
}

double distance_pacman(int x, int y, pacman_t *pacman)
{
	return sqrt (  pow( absolut(pacman->x - x), 2 ) + pow( absolut(pacman->y - y), 2 )  );
}

int main()
{
	xy size;
	pacman_t pacman;
	ghosts_t ghosts;
	char **points = NULL;
	points = init_points(&size, "map.txt", &pacman, &ghosts);
	if(!points)
		return -1;

	initscr();
	cbreak();//strg - c zum beenden des Programms
	noecho();
	curs_set(0);

	int x_max, y_max;
	getmaxyx(stdscr, y_max, x_max);

	WINDOW *game = newwin(size.y, size.x, y_max/2 - size.y/2, x_max/2 - size.x/2);
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

	init_pair(7, COLOR_WHITE, COLOR_BLACK);

//	wbkgd(stdscr, COLOR_PAIR(1));//stdscr blau färben
	
	int run = 1;

	int pressed_key = 0;

	int move_pacman = 0;
	int move_red = 0; //eigentlich aufteilenn in die 4 Geister

	direction_t input = neutral;

	flushinp();

	pacman.lives = 3;//in "init_pacman" oder vergleichbares verschieben

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
		
		move_pacman++;

		if(move_pacman >= pacman.speed) //alle 150ms
		{
			move_pacman = 0;
			
			pacman_kollision(&pacman, &input, points, size, &ghosts);

			mvwprintw(stdscr, 5, 5, "x: %2d y: %2d", pacman.x, pacman.y);//============TEST-AUSGABE===========
			mvwprintw(stdscr, 6, 5, "lives: %1d", pacman.lives);
			mvwprintw(stdscr, 7, 5, "score: %6d", pacman.score);
			mvwprintw(stdscr, 8, 5, "points_collected: %6d", pacman.points_collected);//============TEST-AUSGABE===========
		}
		
		move_red++;
		if(move_red >= ghosts.red.speed)
		{
			move_red = 0;
			//bewege Geister
			move_ghost_red(&ghosts.red, &pacman, points, size);
			//kollision
			if(pacman_geister_kollision(&pacman, &ghosts))
			{
				reset_pacman(&pacman, &input);
				reset_ghosts(&ghosts);
			}
		}
		//====PRINT / AUSGABE====
		werase(game);

		print_board(points, size, game);

		print_pacman(pacman, game);

		print_ghosts(ghosts, game);
		wrefresh(game);
	
		if(pacman.lives < 0)
		{
			run = 0;
		}
	}

	endwin();
	return 0;
}

char **create_points(xy *size)
{
	char **points;
	
	points = (char **) malloc(size->x * sizeof(points)); //sizey viele pointer auf pointer

	if(!points)
		return NULL;
	
	for(int i = 0; i < size->x; ++i) //sizex viele char reservieren für jeden pointer
	{
		points[i] = (char *) malloc(size->y * sizeof(char));
		if(!points[i])
			return NULL;
	}

	return points;
}

char **init_points(xy *size, char* filename, pacman_t *pacman, ghosts_t *ghosts)
{
	FILE *fp;
	fp = fopen(filename, "r");
	if(!fp)
	{
		printf("FEHLER konnte map Datei nicht öffnen!");
		return NULL;
	}
		
	fscanf(fp, "%d", &size->x);
	fscanf(fp, "%d", &size->y);
	fgetc(fp);
	char **points = NULL;
	points = create_points(size);
	if(!points)
	{
		printf("FEHLER konnte Speicher für Spielfeld nicht allokieren!");
		return NULL;
	}

	pacman->lives = 3;
	pacman->score = 0;
	pacman->points_collected = 0;


	for(int i = 0; i < size->y; ++i)
	{
		for(int j = 0; j < size->x; ++j)
		{
			points[j][i] = fgetc(fp);
			switch(points[j][i])
			{
				case '$':
					pacman->x = j;
					pacman->y = i;
					pacman->start_x = j;
					pacman->start_y = i;
					pacman->speed = 15;
					pacman->direction = neutral;	
					points[j][i] = ' ';
					break;
				case 'R':
					ghosts->red.x = j;
					ghosts->red.y = i;
					ghosts->red.start_x = j;
					ghosts->red.start_y = i;
					ghosts->red.state = chase;			
					ghosts->red.direction = left;
					ghosts->red.speed = 20;	
					points[j][i] = ' ';
					break;
				case 'P':
					ghosts->pink.x = j;
					ghosts->pink.y = i;
					ghosts->pink.start_x = j;
					ghosts->pink.start_y = i;
					ghosts->pink.direction = left;
					ghosts->pink.state = idle;
					ghosts->pink.speed = 20;
					points[j][i] = ' ';
					break;
				case 'C':
					ghosts->cyan.x = j;
					ghosts->cyan.y = i;
					ghosts->cyan.start_x = j;
					ghosts->cyan.start_y = i;
					ghosts->cyan.direction = left;
					ghosts->cyan.state = idle;
					ghosts->cyan.speed = 20;
					points[j][i] = ' ';
					break;
				case 'O':
					ghosts->orange.x = j;
					ghosts->orange.y = i;
					ghosts->orange.start_x = j;
					ghosts->orange.start_y = i;
					ghosts->orange.direction = left;
					ghosts->orange.state = idle;
					ghosts->orange.speed = 20;
					points[j][i] = ' ';
					break;
				case '.':
					pacman->points_collected += 1;
					break;
				case 'o':
					pacman->points_collected += 1;
					break;
			} 
		}
		fgetc(fp);
	}
	fclose(fp);
	return points;
}

void print_board(char **points, xy size, WINDOW* win)
{
	for(int i = 0; i < size.y; ++i)
	{
		for(int j = 0; j < size.x; ++j)
		{
			if(points[j][i] == 'W')
			{
				wattron(win, COLOR_PAIR(1));
					mvwprintw(win, i, j, "%c", points[j][i]);
				wattroff(win, COLOR_PAIR(1));
			}
			else
			{
				wattron(win, COLOR_PAIR(7));
					mvwprintw(win, i, j, "%c", points[j][i]);
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
	wattron(win, COLOR_PAIR(3));
	mvwprintw(win, ghosts.red.y, ghosts.red.x, "%c", 'R');
	wattroff(win, COLOR_PAIR(3));
	//pink
	wattron(win, COLOR_PAIR(4));
	mvwprintw(win, ghosts.pink.y, ghosts.pink.x, "%c", 'P');
	wattroff(win, COLOR_PAIR(4));
	//cyan
	wattron(win, COLOR_PAIR(5));
	mvwprintw(win, ghosts.cyan.y, ghosts.cyan.x, "%c", 'C');
	wattroff(win, COLOR_PAIR(5));
	//orange
	wattron(win, COLOR_PAIR(6));
	mvwprintw(win, ghosts.orange.y, ghosts.orange.x, "%c", 'O');
	wattroff(win, COLOR_PAIR(6));		
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

void pacman_kollision(pacman_t *pacman, direction_t *input, char **points, xy size, ghosts_t *ghosts)
{
	int x = next_move(pacman->x, pacman->y, *input, size).x;
	int y = next_move(pacman->x, pacman->y, *input, size).y;

	//input übernehmen?
	if(points[ next_move(pacman->x, pacman->y, *input, size).x ][ next_move(pacman->x, pacman->y, *input,size).y ] != 'W')
	{
		pacman->direction = *input;
	}
	else
	{
		x = next_move(pacman->x, pacman->y, pacman->direction, size).x;
		y = next_move(pacman->x, pacman->y, pacman->direction, size).y;

		if(points[x][y] == 'W')
			return;
	}
	
	//Punkte essen
	if(points[x][y] == '.')
	{
		pacman->score += 100;
		pacman->points_collected -= 1;
		points[x][y] = ' ';
	}
	//enegizer Punkte essen
	if(points[x][y] == 'o')
	{
		pacman->score += 250;
		pacman->points_collected -= 1;
		points[x][y] = ' ';
		//frighten ghosts 
		//geschwindigkeit pacman anheben? 
	}
	//erst bewegen, dann kollision mit Geister bestimmen
	move_pacman(pacman, size);

	//geister kollision
	if(pacman_geister_kollision(pacman, ghosts))
	{
		reset_pacman(pacman, input);
		reset_ghosts(ghosts);
	}	
}

int pacman_geister_kollision(pacman_t *pacman, ghosts_t *ghosts)
{
	if(pacman_geist_kollision(pacman, ghosts->red))
		return 1;
	if(pacman_geist_kollision(pacman, ghosts->pink))
		return 1;
	if(pacman_geist_kollision(pacman, ghosts->orange))
		return 1;
	if(pacman_geist_kollision(pacman, ghosts->cyan))
		return 1;

	return 0;
}

int pacman_geist_kollision(pacman_t *pacman, ghost_t ghost)
{
	if( (ghost.x == pacman->x) && (ghost.y == pacman->y) )
		return 1;
	return 0;
}

void move_ghost_red(ghost_t *red, pacman_t *pacman, char **points, xy size)
{
	//double distance_move = 0.0;
	//double distance_left = 0.0;
	//double distance_right = 0.0;
	xy move;
	xy left;
	xy right;
	switch(red->state)
	{
		case chase:

			move = next_move(red->x, red->y, red->direction, size);
			left = next_move_left(red->x, red->y, red->direction, size);
			right = next_move_right(red->x, red->y, red->direction, size);

		//	get_next_move_ghost(red->x, red->y, red->direction, pacman, points, size);
			
			//==================================================
			/*
			if(points[move.x][move.y] != 'W')
				distance_move = sqrt (  pow( absolut(pacman->x - move.x), 2 ) + pow( absolut(pacman->y - move.y), 2 )  );
			else
				distance_move = 1000;

			if(points[left.x][left.y] != 'W')
				distance_left = sqrt (  pow( absolut(pacman->x - left.x), 2 ) + pow( absolut(pacman->y - left.y), 2 )  );
			else
				distance_left = 1000;

			if(points[right.x][right.y] != 'W')
				distance_right = sqrt (  pow( absolut(pacman->x - right.x), 2 ) + pow( absolut(pacman->y - right.y), 2 )  );
			else
				distance_right = 1000;
			//==================================================
			*/

			if(get_next_move_ghost(red->x, red->y, red->direction, pacman, points, size) == red->direction )
			{
				//gerade aus
				red->x = move.x;
				red->y = move.y;
				return;
			}
			
			if(get_next_move_ghost(red->x, red->y, red->direction, pacman, points, size) == direction_left(red->direction))
			{
				//links
				red->x = left.x;
				red->y = left.y;
				red->direction = direction_left(red->direction);
				return;
			}

			if(get_next_move_ghost(red->x, red->y, red->direction, pacman, points, size) == direction_right(red->direction))
			{
				//rechts
				red->x = right.x;
				red->y = right.y;
				red->direction = direction_right(red->direction);
				return;
			}
			//======================================================
			break;
		case frightened:
			break;
		case scatter:
			break;
		case idle:
			return;
			break;
	}
}

direction_t get_next_move_ghost(int x, int y, direction_t direction, pacman_t *pacman, char **points, xy size)
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

	if(points[move.x][move.y] != 'W')
	{
		distance_move = get_next_next_move_ghost(move.x, move.y, direction, pacman, points, size);
	}
	else
		distance_move = 1000;

	if(points[left.x][left.y] != 'W')
	{
		distance_left = get_next_next_move_ghost(left.x, left.y, direction_left(direction), pacman, points, size);
	}
	else
		distance_left = 1000;

	if(points[right.x][right.y] != 'W')
	{
		distance_right = get_next_next_move_ghost(right.x, right.y, direction_right(direction), pacman, points, size);
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

double get_next_next_move_ghost(int x, int y, direction_t direction, pacman_t *pacman, char **points, xy size)
{
	double distance_move = 0.0;
	double distance_left = 0.0;
	double distance_right = 0.0;
	xy move = next_move(x, y, direction, size);
	xy left = next_move_left(x,y, direction, size);
	xy right = next_move_right(x, y, direction, size);
	
	//=====Distanz======================================
	if(points[move.x][move.y] != 'W')
		distance_move = distance_pacman(move.x, move.y, pacman);
	else
		distance_move = 1000;

	if(points[left.x][left.y] != 'W')
		distance_left =  distance_pacman(left.x, left.y, pacman);
	else
		distance_left = 1000;

	if(points[right.x][right.y] != 'W')
		distance_right =  distance_pacman(right.x, right.y, pacman);
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