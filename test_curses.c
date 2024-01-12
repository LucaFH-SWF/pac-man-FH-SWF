#ifdef __unix__
#include <unistd.h>
#include <ncurses.h>
#include "pacman_linux.h"
#endif
#ifdef WIN32
#include <windows.h>
#include <ncurses/ncurses.h>
#include "pacman_win32.h"
#endif

#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include "pac_types.h" //HEADER Datei mit den benutzten structs und enums

char **create_field(xy size);																//alokieren des Speichers für das Spielfeld
void init_field(char *filename, pacman_t *pacman, ghosts_t *ghosts, game_t *game);			//initialisieren von: Spielfeld, Pacman, Geister
void re_init_field(char *filename, pacman_t *pacman, ghosts_t *ghosts, game_t *game);		//reinitialisieren von: Spielfeld
void reset_pacman(pacman_t *pacman, direction_t *input);									//Pacman auf Startposition setzen und 1 Leben abziehen
void reset_ghosts(ghosts_t *ghosts);														//Geister auf Startposition setzen
void reset_ghost(ghost_t *ghost);															//einen Geist auf Startposition setzen
void frighten_ghosts(ghosts_t *ghosts);														//versetzt alle Geister in frightend
void scatter_ghosts(ghosts_t *ghosts, game_t *game);										//versetzt alle Geister in scatter 
void trap_ghost(ghost_t *ghost);															//einen Geist einsperren
void print_pacman(pacman_t pacman, WINDOW*);												//Ausgabe Pacman
void print_ghosts(ghosts_t ghosts, WINDOW*);												//Ausgabe Geister
void print_board(char **field, xy size, WINDOW*);											//Ausgabe des Spielfeldes
char richtungtochar(direction_t richtung);													//Pfeil Pacman
xy next_move(int x, int y, direction_t direction, xy size);									//gibt die nächste position bei angegebener Richtung zurück
xy next_move_left(int x, int y, direction_t direction, xy size);							//gibt die nächste position links bei angegebener Richtung zurück
xy next_move_right(int x, int y, direction_t direction, xy size);							//gibt die nächste position rechts bei angegebener Richtung zurück
void move_pacman(pacman_t *pacman, xy size);												//Verschiebt die x un y Werte von Pacman in seine Bewegungs Richtung
void pacman_handeling(pacman_t *pacman, direction_t *input, ghosts_t *ghosts, game_t *game);//Kollisions abfrage Pacman
int pacman_geister_kollision(pacman_t *pacman, ghosts_t *ghosts, int *score);				//kollisons abfrage pacman und alle Geister
int pacman_geist_kollision(pacman_t *pacman, ghost_t *ghost, int *score);					//kollisons abfrage pacman und ein Geist
void spawn_ghost(ghost_t *ghost, xy *pos);													//Geister auf dem Spielfeld platzieren
void move_ghost_red(ghost_t *red, pacman_t *pacman, char **field, xy size);					//Bewege den roten Geist
void move_ghost_pink(ghost_t *pink, pacman_t *pacman, char **field, xy size);				//Bewege den pinken Geist
void move_ghost_orange(ghost_t *orange, pacman_t *pacman, char **field, xy size);			//Bewege den orangenen Geist
void move_ghost_cyan(ghost_t *cyan, ghost_t *red, pacman_t *pacman, char **field, xy size); //Bewege den türkisen Geist
void path_ghost_to_xy(ghost_t *ghost, int x, int y, char **field, xy size);					//bewegt einen Geist so das er das angegebene x,y erreicht
xy cyan_chase(pacman_t *pacman, ghost_t red_ghost);											//berechnet das Ziel Feld für den türkisen Geist
direction_t get_next_direction_ghost(int x, int y, direction_t direction, int to_x, int to_y, char **field, xy size);/*Ruft get_next_next_move_ghost bis zu 3 mal auf
																													    und wählt die kleinste Entfernung um die Richtung zu bestimmen.*/
double get_next_next_move_ghost(int x, int y, direction_t direction,int to_x, int to_y, char **field, xy size);		 //wird von get_next_direction_ghost aufgerufen und gibt eine Entfernung zurück
direction_t direction_left(direction_t direction);											//gibt die richtung links von der angegebenen Richtung zurück
direction_t direction_right(direction_t direction);											//gibt die richtung rechts von der angegebenen Richtung zurück
void reverse_direction(direction_t *direction);												//gibt die entgegen gesätzte Richtung zurück
xy get_move_ahead(int x, int y, direction_t direction, int n);								//gibt die x,y Werte zurück die in n bewegungen, bei angegebener Richtung, erreicht wird 

int absolut(int number)																		//Betragsfunktion
{
	if(number > 0)
		return number;
	else
		return ( number * (-1) );
}

double distance_xy(int x, int y, int to_x, int to_y)										//Berechnent den Abstand zwischen zwei Punkten
{
	return sqrt (  pow( absolut(to_x - x), 2 ) + pow( absolut(to_y - y), 2 )  );
}

int main()
{
	game_t game;
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

	if(x_max<game.size.x || y_max<(game.size.y+5))
	{
		endwin();
		printf("Terminal nicht gross genug, min x = %d und min y = %d\n", game.size.x, game.size.y+5);
		printf("erkannte groesse: x = %d und y = %d", x_max, y_max);
		return 1;
	}

	int x_screen = x_max/2 - game.size.x/2;
	int y_srcreen = y_max/2 - game.size.y/2;

	WINDOW *game_win = newwin(game.size.y, game.size.x, y_srcreen+3, x_screen);//spielfeld
	WINDOW *score_win = newwin(3, game.size.x, y_srcreen, x_screen);//extra win für den score
	refresh();

	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);

	start_color();

	init_pair(1, COLOR_BLUE, COLOR_BLUE); 	 //spielfeld
	init_pair(2, COLOR_BLACK, COLOR_YELLOW); //Pacman
	init_pair(3, COLOR_BLACK, COLOR_RED);    //Geist Rot
	init_pair(4, COLOR_BLACK, COLOR_MAGENTA);//Geist Pink
	init_pair(5, COLOR_BLACK, COLOR_CYAN);   //Geist Cyan
	init_pair(6, COLOR_BLACK, COLOR_YELLOW); //Geist "Orange"

	init_pair(7, COLOR_WHITE, COLOR_BLACK);  //spielfeld Dots
	init_pair(8, COLOR_BLUE, COLOR_WHITE);	 //Geister frightened
	init_pair(9, COLOR_WHITE, COLOR_BLUE);   //Geister frightened 2
	
	int run = 1;

	int pressed_key = 0;

	int move_pacman = 0;
	int move_red = 0;
	int move_pink = 0;
	int move_orange = 0;
	int move_cyan = 0;

	int prev_loop = 0;

	direction_t input = neutral;

	flushinp();

	while(run) //action loop
	{ 

		if( timing(&prev_loop) )
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

			// Gödeker | ====PACMAN====

			if(move_pacman >= pacman.speed)
			{
				move_pacman = 0;
				
				pacman_handeling(&pacman, &input, &ghosts, &game);
			}
			
			// Gödeker |===GEISTER===
			if(move_red >= ghosts.red.speed)
			{
				move_red = 0;
				//bewege Geist
				move_ghost_red(&ghosts.red, &pacman, game.field, game.size);
			}

			if(move_pink >= ghosts.pink.speed)
			{
				move_pink = 0;
				//bewege Geist
				move_ghost_pink(&ghosts.pink, &pacman, game.field, game.size);
			}

			if(move_orange >= ghosts.orange.speed)
			{
				move_orange = 0;
				//bewege Geist
				move_ghost_orange(&ghosts.orange, &pacman, game.field, game.size);
			}

			if(move_cyan >= ghosts.cyan.speed)
			{
				move_cyan = 0;
				//bewege Geist
				move_ghost_cyan(&ghosts.cyan, &ghosts.red, &pacman, game.field, game.size);
			}

			// Gödeker |====Spielverlauf-beeinflussend=====
			
			//Red
			if(ghosts.red.state != idle)
			{
				if(ghosts.red.state == frightened)
				{
					if(ghosts.red.frightened_s <= 0)	
					{
						ghosts.red.state = chase;
						ghosts.red.speed = ghosts.red.speed / 2;
					}
				}
			}
			else
			{
				if(ghosts.red.traped <= 0)//spawnt immer sofort
					spawn_ghost(&ghosts.red, &game.sp_ghohsts);
			}

			//Pink
			if(ghosts.pink.state != idle)
			{
				if(ghosts.pink.state == frightened)
				{
					if(ghosts.pink.frightened_s <= 0)	
					{
						ghosts.pink.state = chase;
						ghosts.pink.speed = ghosts.pink.speed / 2;
					}
				}
			}
			else
			{
				if(pacman.dots_collected > 5)//spawnt wenn Pacman 5 dots gesammelt hat
				{
					if(ghosts.pink.traped <= 0)
						spawn_ghost(&ghosts.pink, &game.sp_ghohsts);
				}
			}

			//Cyan
			if(ghosts.cyan.state != idle)
			{
				if(ghosts.cyan.state == frightened)
				{
					if(ghosts.cyan.frightened_s <= 0)	
					{
						ghosts.cyan.state = chase;
						ghosts.cyan.speed = ghosts.cyan.speed / 2;
					}
				}
			}
			else
			{
				if(pacman.dots_collected > 30)//spawnt wenn Pacman 30 dots gesammelt hat
				{
					if(ghosts.cyan.traped <= 0)
						spawn_ghost(&ghosts.cyan, &game.sp_ghohsts);
				}
			}

			//Orange
			if(ghosts.orange.state != idle)
			{
				if(ghosts.orange.state == frightened)
				{
					if(ghosts.orange.frightened_s <= 0)	
					{
						ghosts.orange.state = chase;
						ghosts.orange.speed = ghosts.orange.speed / 2;
					}
				}
			}
			else
			{
				if(game.level > 1 && pacman.dots_collected > (pacman.dots_tocollect/3))//spawnt erst ab dem 2. Level und wenn Pacman 1/3 der dots gesammelt hat
				{
					if(ghosts.orange.traped <= 0)
						spawn_ghost(&ghosts.orange, &game.sp_ghohsts);
				}
			}
			
			//wenn der scatter Timer abgelaufen ist, alle Geister in scatter versetzen
			if(game.scatter_n < 6 && game.scatter <= 0) // Geister scattern 4 mal bevor sie pacman für immer verfolgen
			{
				scatter_ghosts(&ghosts, &game);
			}

			// Gödeker | hoch oder runter zählen von Timern
			//===============================================
			move_pacman++;
			move_red++;
			move_pink++;
			move_orange++;
			move_cyan++;
			game.scatter -=1;

			//frightened
			if(ghosts.red.frightened_s > 0)
				ghosts.red.frightened_s--;

			if(ghosts.pink.frightened_s > 0)
				ghosts.pink.frightened_s--;

			if(ghosts.orange.frightened_s > 0)
				ghosts.orange.frightened_s--;

			if(ghosts.cyan.frightened_s > 0)
				ghosts.cyan.frightened_s--;

			//Geister traped
			if(ghosts.red.traped >0)
				ghosts.red.traped--;

			if(ghosts.pink.traped >0)
				ghosts.pink.traped--;

			if(ghosts.orange.traped >0)
				ghosts.orange.traped--;

			if(ghosts.cyan.traped >0)
				ghosts.cyan.traped--;
			//===============================================

			//kollision
			if(pacman_geister_kollision(&pacman, &ghosts, &game.score))
			{
				reset_pacman(&pacman, &input);
				reset_ghosts(&ghosts);
				game.scatter_n = 0;
				game.scatter = 1250;
			}
			
			// Önder | ====PRINT / AUSGABE====
			werase(game_win);
			werase(score_win);

			print_board(game.field, game.size, game_win);

			print_pacman(pacman, game_win);

			print_ghosts(ghosts, game_win);

			wrefresh(game_win);

			//Score Anzeige
			mvwprintw(score_win, 0, 0, "LEVEL: %d", game.level);  mvwprintw(score_win, 0, 17, "EXIT: q");
			mvwprintw(score_win, 1, 0, "SCORE: %08d", game.score); mvwprintw(score_win, 1, 16, "LIVES: %d", pacman.lives);

			wrefresh(score_win);
			
			// Gödeker | Game Over und Next-Level bedingungen
			if(pacman.dots_tocollect == pacman.dots_collected)//Next-Level
			{
				wattron(game_win, COLOR_PAIR(2));
				mvwprintw(game_win, game.size.y/2, game.size.x/2-4, "READY !");
				wattroff(game_win, COLOR_PAIR(2));
				wrefresh(game_win);
				sleep(2);
				game.level += 1;
				pacman.lives +=1;
				reset_ghosts(&ghosts);
				reset_pacman(&pacman, &input);
				pacman.dots_collected = 0;
				pacman.dots_tocollect = 0;
				re_init_field("map.txt", &pacman, &ghosts, &game);
			}

			if(pacman.lives < 0) //GAME OVER
			{
				wattron(game_win, COLOR_PAIR(3));
				mvwprintw(game_win, game.size.y/2-2, game.size.x/2-5, "GAME OVER");
				mvwprintw(game_win, game.size.y/2-1, game.size.x/2-8, "SCORE: %08d", game.score);
				wattroff(game_win, COLOR_PAIR(3));
				wrefresh(game_win);
				sleep(3);
				pacman.lives +=1;
				reset_ghosts(&ghosts);
				reset_pacman(&pacman, &input);
				pacman.dots_collected = 0;
				pacman.dots_tocollect = 0;
				re_init_field("map.txt", &pacman, &ghosts, &game);
			}
			
		}
	}

	endwin();
	return 0;
}

//ein char ** feld auf dem heap allokieren und zurück geben
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

//ließt die "map.txt" Datei aus, und initialisiert die Werte entsprechend
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

	//Werte sind immer gleich
	pacman->lives = 3;
	pacman->dots_collected = 0;
	pacman->dots_tocollect = 0;
	game->score = 0;
	game->level = 1;
	game->scatter_n = 0;
	game->scatter = 1000;

	//Werte mit Datei inhalt initialisieren
	for(int i = 0; i < game->size.y; ++i)
	{
		for(int j = 0; j < game->size.x; ++j)
		{
			game->field[j][i] = fgetc(fp);
			switch(game->field[j][i])
			{
				case '$':	// $ ist Pacman
					pacman->x = j;
					pacman->y = i;
					pacman->start_x = j;//start Position
					pacman->start_y = i;
					pacman->speed = 15;
					pacman->direction = neutral;	
					game->field[j][i] = ' ';
					break;
				case 'S':	// S ist der Startpunkt für die Geister
					game->sp_ghohsts.x = j;
					game->sp_ghohsts.y = i;	
					game->field[j][i] = ' ';
					break;
				case 'R': // R ist der rote Geist
					ghosts->red.x = j;
					ghosts->red.y = i;
					ghosts->red.start_x = j;
					ghosts->red.start_y = i;
					ghosts->red.state = idle;			
					ghosts->red.direction = left;
					ghosts->red.speed = 20;
					ghosts->red.traped = 0;
					ghosts->red.frightened_s = 0;
					game->field[j][i] = ' ';
					break;
				case 'P':	// P ist der pinke Geist
					ghosts->pink.x = j;
					ghosts->pink.y = i;
					ghosts->pink.start_x = j;
					ghosts->pink.start_y = i;
					ghosts->pink.direction = left;
					ghosts->pink.state = idle;
					ghosts->pink.speed = 20;
					ghosts->pink.traped = 0;
					ghosts->pink.frightened_s = 0;
					game->field[j][i] = ' ';
					break;
				case 'C': // C ist der türkise Geist
					ghosts->cyan.x = j;
					ghosts->cyan.y = i;
					ghosts->cyan.start_x = j;
					ghosts->cyan.start_y = i;
					ghosts->cyan.direction = left;
					ghosts->cyan.state = idle;
					ghosts->cyan.speed = 20;
					ghosts->cyan.traped = 0;
					ghosts->cyan.frightened_s = 0;
					game->field[j][i] = ' ';
					break;
				case 'O': // O ist der orangene Geist
					ghosts->orange.x = j;
					ghosts->orange.y = i;
					ghosts->orange.start_x = j;
					ghosts->orange.start_y = i;
					ghosts->orange.direction = left;
					ghosts->orange.state = idle;
					ghosts->orange.speed = 20;
					ghosts->orange.traped = 0;
					ghosts->orange.frightened_s = 0;
					game->field[j][i] = ' ';
					break;
				case '.': // . ist ein Dot
					pacman->dots_tocollect += 1;
					break;
				case 'o': // o ist ein "enegizer"
					pacman->dots_tocollect += 1;
					break;
			} 
		}
		fgetc(fp);
	}
	fclose(fp);
	return;
}

//gibt das Spielfeld mit den entsprechenden Farben, auf dem angegebenen ncurses screen, aus
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

//gibt pacman, auf dem angegebenen ncurses screen, aus
void print_pacman(pacman_t pacman, WINDOW* win)
{
	wattron(win, COLOR_PAIR(2));
	mvwprintw(win, pacman.y, pacman.x, "%c", richtungtochar(pacman.direction));
	wattroff(win, COLOR_PAIR(2));
}

//gibt den char zurück, der für Pacman benutzt werden soll 
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

//gibt alle Geister aus, es wird zwischen frightened und nicht frightened unterschieden
void print_ghosts(ghosts_t ghosts, WINDOW* win)
{
	//red
	if(ghosts.red.state != frightened)
	{
		//normale Ausgabe
		wattron(win, COLOR_PAIR(3));
		mvwprintw(win, ghosts.red.y, ghosts.red.x, "%c", 'R');
		wattroff(win, COLOR_PAIR(3));
	}
	else
	{
		//Ausgabe wenn der Geist frightened ist
		if(ghosts.red.frightened_s < 125)
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
		else
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.red.y, ghosts.red.x, "%c", 'R');
			wattroff(win, COLOR_PAIR(8));
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
		if(ghosts.pink.frightened_s < 125)
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
		else
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.pink.y, ghosts.pink.x, "%c", 'P');
			wattroff(win, COLOR_PAIR(8));
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
		if(ghosts.cyan.frightened_s < 150)
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
		else
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.cyan.y, ghosts.cyan.x, "%c", 'C');
			wattroff(win, COLOR_PAIR(8));
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
		if(ghosts.orange.frightened_s < 125)
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
		else
		{
			wattron(win, COLOR_PAIR(8));
			mvwprintw(win, ghosts.orange.y, ghosts.orange.x, "%c", 'O');
			wattroff(win, COLOR_PAIR(8));
		}
	}
}

//setzt pacmans Werte und den input zurück 
void reset_pacman(pacman_t *pacman, direction_t *input)
{
	pacman->x = pacman->start_x;
	pacman->y = pacman->start_y;
	pacman->dots_tocollect -= pacman->dots_collected;
	pacman->dots_collected = 0;
	pacman->direction = neutral;
	pacman->lives -= 1;
	*input = neutral;
}

void reset_ghosts(ghosts_t *ghosts)
{
	reset_ghost(&ghosts->red);
	reset_ghost(&ghosts->pink);
	reset_ghost(&ghosts->orange);
	reset_ghost(&ghosts->cyan);
}

//setzt die Werte des Geistes zurück
void reset_ghost(ghost_t *ghost)
{
	ghost->x = ghost->start_x;
	ghost->y = ghost->start_y;
	ghost->frightened_s = 0;
	ghost->traped = 0;
	ghost->direction = left;
	if(ghost->state == frightened)
		ghost->speed = ghost->speed / 2;	
	ghost->state = idle;
}

//bewegt Pacman
void move_pacman(pacman_t *pacman, xy size)
{
	pacman->x = next_move(pacman->x, pacman->y, pacman->direction, size).x;
	pacman->y = next_move(pacman->x, pacman->y, pacman->direction, size).y;
}

// Gödeker | gibt ein xy Tupel des nächsten Felds zurück, das wenn man in die angegebene Richtung geht erreicht
xy next_move(int x, int y, direction_t direction, xy size)
{
	xy pos;
	pos.x = x;
	pos.y = y;
	switch(direction) //Richtung in x und y bewegung übersätzen
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
	//auf außerhalb des Bereichs überprüfen:
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

	//falls innerhalb des Bereichs:
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

//Gödeker | bewegt pacman und 
void pacman_handeling(pacman_t *pacman, direction_t *input, ghosts_t *ghosts, game_t *game)
{
	int x = next_move(pacman->x, pacman->y, *input, game->size).x;
	int y = next_move(pacman->x, pacman->y, *input, game->size).y;

	//input übernehmen?
	if(game->field[ x ][ y ] != 'W')
	{
		pacman->direction = *input; //input über nommen
	}
	else
	{
		//input nicht übernehmen und vorherige richtung beibehalten
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
		/*enegizer geben 250 score und versetzen die Geister in frightened,
		   sodass Pacman sie essen kann*/
		game->score += 250;
		pacman->dots_collected += 1;
		game->field[x][y] = ' ';

		frighten_ghosts(ghosts);
	}
	//verändern der x und y Werte von Pacman 
	move_pacman(pacman, game->size);
}

// Gödeker
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

// Gödeker | überprüft auf kolisionen von Pacman und Geistern
int pacman_geist_kollision(pacman_t *pacman, ghost_t *ghost, int *score)
{
	if( (ghost->x == pacman->x) && (ghost->y == pacman->y) )//kollision?
	{
		if(ghost->state != frightened)
			return 1;
		else
		{
			//wenn der Geist frightened ist, kann Pacman ihn essen und bekommt 200 score
			*score = *score + 200;
			trap_ghost(ghost);
		}
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
	xy pos;
	switch(pink->state)
	{
		case chase:
			pos = get_move_ahead(pacman->x, pacman->y, pacman->direction, 4);
			path_ghost_to_xy(pink, pos.x, pos.y, field, size);
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

void move_ghost_cyan(ghost_t *cyan, ghost_t *red, pacman_t *pacman, char **field, xy size)
{
	switch(cyan->state)
	{
		case chase:

		path_ghost_to_xy(cyan, cyan_chase(pacman, *red).x, cyan_chase(pacman, *red).y, field, size);
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
//Mike Esteves
//Sperrt Geister in den Kasten in der mitte der Map, tut sie auf ihre Startposition und setzt ihren Speed zurück
void trap_ghost(ghost_t *ghost)
{
	ghost->x = ghost->start_x;
	ghost->y = ghost->start_y;
	ghost->traped = 400;
	if(ghost->state == frightened)
		ghost->speed = ghost->speed / 2;
	ghost->state = idle;
}
//Mike Esteves
//Frightened Verhalten für Geister. Sie sind langsamer, reversen einmal ihre Richtung und bewegen sich dann zufällig. Mit Timer
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
		ghosts->red.frightened_s = 500;
	}
	if(ghosts->pink.state != idle)
	{
		ghosts->pink.state = frightened;
		ghosts->pink.speed = ghosts->pink.speed * 2;
		ghosts->pink.frightened_s = 500;
	}
	if(ghosts->orange.state != idle)
	{
		ghosts->orange.state = frightened;
		ghosts->orange.speed = ghosts->orange.speed * 2;
		ghosts->orange.frightened_s = 500;
	}
	if(ghosts->cyan.state != idle)
	{
		ghosts->cyan.state = frightened;
		ghosts->cyan.speed = ghosts->cyan.speed * 2;
		ghosts->cyan.frightened_s = 500;
	}
}
//Mike Esteves
//Wechselt State eines Geists auf Chase und setzt diesen auf die übergeben Position
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
//Mike Esteves
//Chase Verhalten für den blauen Geist. Bewegt sich in Relation zu der Position von dem roten Geist zu Pacman.
xy cyan_chase(pacman_t *pacman, ghost_t red_ghost)
{
	xy_double pos;
	pos.x = pacman->x;
	pos.y = pacman->y;
	xy_double vec;
	xy result;

	pos = get_move_ahead(pos.x, pos.y, pacman->direction, 2);

	//If Schaltung um die Position von Rot zu Pacman festzustellen. Zieht einen Vektor von Rot auf einen Punk der Pacman 2 Felder vorraus ist.
	//Verdoppelt dann die länge um das Zielfeld für Blau zu finden.
	if(red_ghost.x < pos.x)
	{
		vec.x = pos.x - red_ghost.x;
		vec.x = vec.x * 1.125;
		pos.x += vec.x;
	}
	else
	{
		vec.x = red_ghost.x - pos.x;
		vec.x = vec.x * 1.125;
		pos.x -= vec.x;
	}
	if(red_ghost.y < pos.y){
		vec.y = pos.y - red_ghost.y;
		vec.y = vec.y * 1.125;
		pos.y += vec.y;
	}
	else
	{
		vec.y = red_ghost.y - pos.y;
		vec.y = vec.y * 1.125;
		pos.y -= vec.y;
	}
	result.x = pos.x;
	result.y = pos.y;
	return result;
}
//Mike Esteves
//Berechnet ein Feld welches n Felder vor der Position einer gewählten Spielfigur ist.
xy get_move_ahead(int x, int y, direction_t direction, int n)
{
	xy pos;
	pos.x = x;
	pos.y = y;
	switch(direction)
	{
		case up:
			pos.y -= n;
			break;
		case down:
			pos.y += n;
			break;
		case left:
			pos.x -= n;
			break;
		case right:
			pos.x += n;
			break;
		default:
			break;
	}
	return pos;
}
//Mike Esteves
//Scatter Verhalten der Geister - Sie gehen in ihre Home Corner zurück
void scatter_ghosts(ghosts_t *ghosts, game_t *game)
{
	if(!(game->scatter_n % 2))
	{
		game->scatter = 400;
		game->scatter_n += 1;
		reverse_direction(&ghosts->red.direction);
		reverse_direction(&ghosts->pink.direction);
		reverse_direction(&ghosts->orange.direction);
		reverse_direction(&ghosts->cyan.direction);
	}
	else
	{
		game->scatter = 1250;
		game->scatter_n += 1;
	}

	if(ghosts->red.state != idle && ghosts->red.state != frightened)
	{
		if(ghosts->red.state == scatter)
			ghosts->red.state = chase;
		else
			ghosts->red.state = scatter;
	}
	if(ghosts->pink.state != idle && ghosts->pink.state != frightened)
	{
		if(ghosts->pink.state == scatter)
			ghosts->pink.state = chase;
		else
			ghosts->pink.state = scatter;
	}
	if(ghosts->orange.state != idle && ghosts->orange.state != frightened)
	{
		if(ghosts->orange.state == scatter)
			ghosts->orange.state = chase;
		else
			ghosts->orange.state = scatter;
	}
	if(ghosts->cyan.state != idle && ghosts->cyan.state != frightened)
	{
		if(ghosts->cyan.state == scatter)
			ghosts->cyan.state = chase;
		else
			ghosts->cyan.state = scatter;
	}
}

void re_init_field(char *filename, pacman_t *pacman, ghosts_t *ghosts, game_t *game)//reinitialisieren von: Spielfeld
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

	if(pacman->lives < 0)//GAME OVER
	{
		pacman->lives = 3;
		game->level = 1;
		game->score = 0;
	}
	else
	{
		pacman->lives += 1;
	}
	pacman->dots_collected = 0;
	pacman->dots_tocollect = 0;
	game->scatter_n = 0;
	game->scatter = 1000;


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
					pacman->speed = 16 + ((game->level-1) * 1);
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
					ghosts->red.state = idle;			
					ghosts->red.direction = left;
					ghosts->red.speed = 20 - ((game->level-1) * 2);
					ghosts->red.traped = 0;
					ghosts->red.frightened_s = 0;
					game->field[j][i] = ' ';
					break;
				case 'P':
					ghosts->pink.x = j;
					ghosts->pink.y = i;
					ghosts->pink.direction = left;
					ghosts->pink.state = idle;
					ghosts->pink.speed = 20 - ((game->level-1) * 2);
					ghosts->pink.traped = 0;
					ghosts->pink.frightened_s = 0;
					game->field[j][i] = ' ';
					break;
				case 'C':
					ghosts->cyan.x = j;
					ghosts->cyan.y = i;
					ghosts->cyan.direction = left;
					ghosts->cyan.state = idle;
					ghosts->cyan.speed = 20 - ((game->level-1) * 2);
					ghosts->cyan.traped = 0;
					ghosts->cyan.frightened_s = 0;
					game->field[j][i] = ' ';
					break;
				case 'O':
					ghosts->orange.x = j;
					ghosts->orange.y = i;
					ghosts->orange.direction = left;
					ghosts->orange.state = idle;
					ghosts->orange.speed = 20 - ((game->level-1) * 2);
					ghosts->orange.traped = 0;
					ghosts->orange.frightened_s = 0;
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