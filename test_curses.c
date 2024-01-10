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
xy next_move(int x, int y, direction_t direction);//gibt die nächste position bei angegebener Richtung zurück
xy next_move_left(int x, int y, direction_t direction);//gibt die nächste position links bei angegebener Richtung zurück
xy next_move_right(int x, int y, direction_t direction);//gibt die nächste position rechts bei angegebener Richtung zurück
void move_pacman(pacman_t *pacman);//Verschiebt die x un y Werte von Pacman in seine Bewegungs Richtung
void pacman_kollision(pacman_t *pacman, direction_t *input, char **points, xy size, ghosts_t *ghosts);//Kollisions abfrage Pacman
int oob(pacman_t *pacman, int x, int y, xy size);//pacman außerhalb des Spielfeldes ( oob = out of bounds )
int pacman_geister_kollision(pacman_t *pacman, ghosts_t *ghosts);//kollisons abfrage pacman und alle Geister
int pacman_geist_kollision(pacman_t *pacman, ghost_t ghost);//kollisons abfrage pacman und ein Geist
void move_ghost_red(ghost_t *red);

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
			move_ghost_red(&ghosts.red);
			//kollision geist pacman?
			//kollision -> game over
			//keine kollision -> geist bewegen
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
					points[j][i] = ' ';
					break;
				case 'P':
					ghosts->pink.x = j;
					ghosts->pink.y = i;
					ghosts->pink.start_x = j;
					ghosts->pink.start_y = i;
					ghosts->pink.state = idle;
					points[j][i] = ' ';
					break;
				case 'C':
					ghosts->cyan.x = j;
					ghosts->cyan.y = i;
					ghosts->cyan.start_x = j;
					ghosts->cyan.start_y = i;
					ghosts->cyan.state = idle;
					points[j][i] = ' ';
					break;
				case 'O':
					ghosts->orange.x = j;
					ghosts->orange.y = i;
					ghosts->orange.start_x = j;
					ghosts->orange.start_y = i;
					ghosts->orange.state = idle;
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
	ghosts->red.speed = 12;

	ghosts->pink.x = ghosts->pink.start_x;
	ghosts->pink.y = ghosts->pink.start_y;
	ghosts->pink.state = idle;
	ghosts->pink.speed = 12;

	ghosts->orange.x = ghosts->orange.start_x;
	ghosts->orange.y = ghosts->orange.start_y;
	ghosts->orange.state = idle;
	ghosts->orange.speed = 12;

	ghosts->cyan.x = ghosts->cyan.start_x;
	ghosts->cyan.y = ghosts->cyan.start_y;
	ghosts->cyan.state = idle;
	ghosts->cyan.speed = 12;
}

void move_pacman(pacman_t *pacman)
{
	pacman->x = next_move(pacman->x, pacman->y, pacman->direction).x;
	pacman->y = next_move(pacman->x, pacman->y, pacman->direction).y;
}

xy next_move(int x, int y, direction_t direction)
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
	return pos;
}

xy next_move_left(int x, int y, direction_t direction)
{
	switch(direction)
	{
		case up:
			return next_move(x, y, left);
			break;
		case down:
			return next_move(x, y, right);
			break;
		case left:
			return next_move(x, y, right);
			break;
		case right:
			return next_move(x, y, up);
			break;
		default:
			return next_move(x,y,direction);
			break;
		
	}
}
xy next_move_right(int x, int y, direction_t direction)
{
	switch(direction)
	{
		case up:
			return next_move(x, y, right);
			break;
		case down:
			return next_move(x, y, left);
			break;
		case left:
			return next_move(x, y, up);
			break;
		case right:
			return next_move(x, y, down);
			break;
		default:
			return next_move(x,y,direction);
			break;
		
	}
}

void pacman_kollision(pacman_t *pacman, direction_t *input, char **points, xy size, ghosts_t *ghosts)
{
	int x = next_move(pacman->x, pacman->y, *input).x;
	int y = next_move(pacman->x, pacman->y, *input).y;

	//oob handeling
	if(oob(pacman, x, y, size))
		return;

	//input übernehmen?
	if(points[ next_move(pacman->x, pacman->y, *input).x ][ next_move(pacman->x, pacman->y, *input).y ] != 'W')
	{
		pacman->direction = *input;
	}
	else
	{
		x = next_move(pacman->x, pacman->y, pacman->direction).x;
		y = next_move(pacman->x, pacman->y, pacman->direction).y;
		//oob 2 mit nicht geänderter richtung
		if(oob(pacman, x, y, size))
			return;

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
	move_pacman(pacman);

	//geister kollision
	if(pacman_geister_kollision(pacman, ghosts))
	{
		reset_pacman(pacman, input);
		reset_ghosts(ghosts);
	}	
}

int oob(pacman_t *pacman, int x, int y, xy size)
{
	if(x>size.x-1)
		{
			pacman->x = 0;
			return 1;
		}
		if(y>size.y-1)
		{
			pacman->y = 0;
			return 1;
		}
		if(x<0)
		{
			pacman->x = size.x-1;
			return 1;
		}
		if(y<0)
		{
			pacman->y = size.y;
			return 1;
		}
		return 0;
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

void move_ghost_red(ghost_t *red)
{
	switch(red->state)
	{
		case chase:
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