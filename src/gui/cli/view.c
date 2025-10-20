#include "view.h"

#include <string.h>

static WINDOW *win_board;
static WINDOW *win_info;

void init_terminal() {
  initscr();
  cbreak();
  noecho();
  curs_set(FALSE);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  start_color();
  init_colors();

  win_board = newwin(BOARD_HEIGHT + 2, BOARD_WIDTH * 2 + 2, 1, 1);
  win_info = newwin(BOARD_HEIGHT + 2, 20, 1, BOARD_WIDTH * 2 + 4);

  wbkgd(win_board, COLOR_PAIR(8));
  wbkgd(win_info, COLOR_PAIR(8));
}

void cleanup_terminal() {
  delwin(win_board);
  delwin(win_info);
  endwin();
}

void draw_game(const GameData_t *game) {
  werase(win_board);
  werase(win_info);

  box(win_board, 0, 0);
  box(win_info, 0, 0);

  draw_board(game);
  draw_info_panel(game);

  if (game->info.pause) {
    draw_overlay("PAUSE");
  } else if (game->state == GameOver) {
    draw_overlay("GAME OVER");
  }

  wnoutrefresh(win_board);
  wnoutrefresh(win_info);
  doupdate();
}

void init_colors() {
  init_pair(1, COLOR_CYAN, COLOR_CYAN);
  init_pair(2, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(3, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(4, COLOR_WHITE, COLOR_WHITE);
  init_pair(5, COLOR_BLUE, COLOR_BLUE);
  init_pair(6, COLOR_GREEN, COLOR_GREEN);
  init_pair(7, COLOR_RED, COLOR_RED);

  init_pair(8, COLOR_WHITE, COLOR_BLACK);
}

void draw_board(const GameData_t *game) {
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (game->board[y][x] != 0) {
        wattron(win_board, COLOR_PAIR(game->board[y][x]));
        mvwprintw(win_board, y + 1, x * 2 + 1, "  ");
        wattroff(win_board, COLOR_PAIR(game->board[y][x]));
      }
    }
  }

  if (game->state == Moving || game->state == Shifting) {
    wattron(win_board, COLOR_PAIR(game->current_piece.color_index));
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        if (game->current_piece.shape[i][j] && game->current_piece.y + i >= 0) {
          mvwprintw(win_board, game->current_piece.y + i + 1,
                    (game->current_piece.x + j) * 2 + 1, "  ");
        }
      }
    }
    wattroff(win_board, COLOR_PAIR(game->current_piece.color_index));
  }
}

void draw_info_panel(const GameData_t *game) {
  wattron(win_info, COLOR_PAIR(8));

  mvwprintw(win_info, 2, 2, "Score: %d", game->info.score);
  mvwprintw(win_info, 3, 2, "High:  %d", game->info.high_score);
  mvwprintw(win_info, 4, 2, "Level: %d", game->info.level);

  mvwprintw(win_info, 7, 2, "Next:");

  int next_color = game->next_piece_index + 1;
  wattron(win_info, COLOR_PAIR(next_color));
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (FIGURES[game->next_piece_index][i][j]) {
        mvwprintw(win_info, i + 9, j * 2 + 5, "  ");
      }
    }
  }
  wattroff(win_info, COLOR_PAIR(next_color));

  wattroff(win_info, COLOR_PAIR(8));
}

void draw_overlay(const char *message) {
  int len = strlen(message);
  int y = (BOARD_HEIGHT + 2) / 2 - 1;
  int x = (BOARD_WIDTH * 2 + 2 - len) / 2;

  wattron(win_board, COLOR_PAIR(8));
  mvwprintw(win_board, y, x, message);
  wattroff(win_board, COLOR_PAIR(8));
}