#ifndef GUI_CLI_VIEW_H
#define GUI_CLI_VIEW_H

#include <ncurses.h>

#include "brickgame/tetris/tetris.h"

void init_terminal();
void cleanup_terminal();

void draw_game(const GameData_t *game);

void init_colors();
void draw_board(const GameData_t *game);
void draw_info_panel(const GameData_t *game);
void draw_overlay(const char *message);

#endif