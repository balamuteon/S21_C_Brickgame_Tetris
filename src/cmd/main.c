#include "main.h"

int main() {
  GameData_t game;

  initialize_game(&game);
  init_terminal();

  while (game.state != GameOver) {
    UserAction_t action = get_user_action(getch());
    apply_user_action(&game, action);
    update_game_state(&game);
    draw_game(&game);
    usleep(40000);
  }

  cleanup_terminal();
  save_high_score(game.info.high_score);
  printf("Game Over! Your score: %d\n", game.info.score);
  printf("High Score: %d\n", game.info.high_score);

  return 0;
}