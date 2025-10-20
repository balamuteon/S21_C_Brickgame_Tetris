#ifndef BRICKGAME_TETRIS_H
#define BRICKGAME_TETRIS_H

#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

extern const int FIGURES[7][4][4];

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

#define PTS_TILL_LVLUP 600
#define MAX_LEVEL 10

#define COLOR_I 1
#define COLOR_O 2
#define COLOR_T 3
#define COLOR_L 4
#define COLOR_J 5
#define COLOR_S 6
#define COLOR_Z 7

typedef enum {
  ActionNone,
  ActionStart,
  ActionPause,
  ActionTerminate,
  ActionMoveLeft,
  ActionMoveRight,
  ActionMoveDown,
  ActionRotate
} UserAction_t;

typedef enum {
  Start,
  Spawn,
  Moving,
  Shifting,
  Attaching,
  GameOver
} GameState_t;

typedef struct {
  int shape[4][4];
  int x;
  int y;
  int color_index;
} CurrentPiece_t;

typedef struct {
  int score;
  int high_score;
  int level;
  int speed;
  bool pause;
} GameInfo_t;

typedef struct {
  long ticker;
  int speed_threshold;
} Timer_t;

typedef struct {
  int board[BOARD_HEIGHT][BOARD_WIDTH];
  int next_piece_index;
  GameInfo_t info;
  GameState_t state;
  CurrentPiece_t current_piece;
  Timer_t timer;
} GameData_t;

void initialize_game(GameData_t *game);
void update_game_state(GameData_t *game);

void apply_user_action(GameData_t *game, UserAction_t action);
UserAction_t get_user_action(int key);

int generate_new_shape();
void rotate_piece(GameData_t *game);
void imprint_piece_to_board(GameData_t *game);
void move_piece(GameData_t *game, int dx, int dy);
bool spawn_new_piece(GameData_t *game);
bool check_collision(const GameData_t *game);

int clear_lines(GameData_t *game);
void update_level(GameInfo_t *info);
void add_score(GameInfo_t *info, int cleared_lines);
void process_scoring_and_levelup(GameData_t *game);

int load_high_score();
void save_high_score(int score);

#endif