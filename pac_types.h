#ifndef pacman_types
#define pacman_types

// definiert die Richtungen und Zustände (hoch, runter, verfolgen..)
typedef enum {up, right, down, left, neutral} direction_t;
typedef enum {chase, frightened, scatter, idle} state_t;

// struktur für die Koordinaten in 2D (integer)
struct xy{
    int x;
    int y;
};

// struktur für die Koordinaten in 2D (double)
typedef struct{
    double x;
    double y;
} xy_double;

// struktur für ein Geist mit Position (x, y), Startposition, Geschwindigkeit und anderen Eigenschaften
struct ghost{
    int x;
    int y;
    int start_x;
    int start_y;
    state_t state;
    direction_t direction;
    int speed;
    int traped;
    int frightened_s;
};

// umbennenung von struktur für bessere verständlichkeit
typedef struct xy xy;
typedef struct ghost ghost_t;

// definition der struktur für alle geister im spiel
typedef struct{
    ghost_t red;
    ghost_t pink;
    ghost_t orange;
    ghost_t cyan;
} ghosts_t;

// definition einer struktur für den Pacman-Charakter (Leben, Punktzahl..)
typedef struct{
    int lives;
    int score;
    int dots_collected;
    int dots_tocollect;
    int x;
    int y;
    int start_x;
    int start_y;
    direction_t direction;
    int speed;
}pacman_t;

// struktur für das Spiel
typedef struct{
    char** field;        // Spielfeld in 2D-Array von Zeichen
    xy size;        	 // Größe vom Spielfeld in x und y Koordinaten
    int score;        	 // Punktzahl im Spiel
    xy sp_ghohsts;       // Startposition der Geister
    int level;           // Das aktuelle Spiellevel 
    int scatter;         // Timer für Dauer des Streuens der Geister
    int scatter_n;       // Anzahl wie oft das Streuen stattgefunden hat
}game_t;

#endif