#include "brickgame/tetris/tetris.h"

const int FIGURES[7][4][4] = {
    {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}},  // I
    {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},  // O
    {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},  // T
    {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},  // L
    {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},  // J
    {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},  // S
    {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}   // Z
};

/**
 * @brief Генерирует индекс для следующей случайной фигуры.
 *
 * @return int Индекс фигуры в массиве FIGURES (число от 0 до 6).
 */
int generate_new_shape() {
  return rand() % 7;
  // return 0;
}

/**
 * @brief Создает новую падающую фигуру вверху экрана.
 *
 * Копирует следующую фигуру в текущую, устанавливает начальные координаты
 * и проверяет на мгновенную коллизию (условие проигрыша).
 * @param game Указатель на главную структуру данных игры.
 * @return true Если спавн прошел успешно.
 * @return false Если произошла коллизия (игра окончена).
 */
bool spawn_new_piece(GameData_t *game) {
  memcpy(game->current_piece.shape, FIGURES[game->next_piece_index],
         sizeof(int) * 16);

  game->current_piece.x = BOARD_WIDTH / 2 - 2;
  game->current_piece.y = -2;
  game->current_piece.color_index = game->next_piece_index + 1;
  game->next_piece_index = generate_new_shape();

  return !check_collision(game);
}

/**
 * @brief Сдвигает текущую фигуру на заданное смещение (dx, dy).
 *
 * Отменяет движение, если оно приводит к столкновению.
 * @param game Указатель на главную структуру данных игры.
 * @param dx Смещение по оси X (например, -1 для движения влево).
 * @param dy Смещение по оси Y (например, 1 для движения вниз).
 */
void move_piece(GameData_t *game, int dx, int dy) {
  CurrentPiece_t temp = game->current_piece;
  game->current_piece.x += dx;
  game->current_piece.y += dy;

  if (check_collision(game)) {
    game->current_piece = temp;  // Отменяем движение при коллизии
  }
}

/**
 * @brief Проверяет наличие столкновений для текущей фигуры.
 * * @param game Указатель на главную структуру данных игры.
 * @return true Если есть столкновение (со стеной, полом или другим блоком).
 * @return false Если столкновений нет.
 */
bool check_collision(const GameData_t *game) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (game->current_piece.shape[i][j]) {
        int board_x = game->current_piece.x + j;
        int board_y = game->current_piece.y + i;
        if (board_x < 0 || board_x >= BOARD_WIDTH || board_y >= BOARD_HEIGHT)
          return true;
        if (board_y >= 0 && game->board[board_y][board_x]) return true;
      }
    }
  }
  return false;
}

/**
 * @brief "Впечатывает" текущую падающую фигуру в игровое поле.
 *
 * Копирует блоки из `current_piece.shape` в `game->board`, делая
 * фигуру частью "стакана".
 * @param game Указатель на главную структуру данных игры.
 */
void imprint_piece_to_board(GameData_t *game) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (game->current_piece.shape[i][j]) {
        int board_y = game->current_piece.y + i;
        int board_x = game->current_piece.x + j;
        if (board_y >= 0 && board_y < BOARD_HEIGHT && board_x >= 0 &&
            board_x < BOARD_WIDTH) {
          game->board[board_y][board_x] = game->current_piece.color_index;
        }
      }
    }
  }
}

/**
 * @brief Ищет, очищает заполненные линии и сдвигает поле вниз.
 *
 * @param game Указатель на главную структуру данных игры.
 * @return int Количество очищенных линий.
 */
int clear_lines(GameData_t *game) {
  int cleared_lines = 0;
  for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
    bool line_is_full = true;
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (game->board[y][x] == 0) {
        line_is_full = false;
        break;
      }
    }

    if (line_is_full) {
      cleared_lines++;
      for (int move_y = y; move_y > 0; move_y--) {
        memcpy(game->board[move_y], game->board[move_y - 1],
               sizeof(int) * BOARD_WIDTH);
      }
      memset(game->board[0], 0, sizeof(int) * BOARD_WIDTH);
      y++;
    }
  }
  return cleared_lines;
}

/**
 * @brief Функция-оркестратор для подсчета очков и повышения уровня.
 *
 * Вызывает clear_lines, и если линии были очищены, вызывает
 * add_score и update_level.
 * @param game Указатель на главную структуру данных игры.
 */
void process_scoring_and_levelup(GameData_t *game) {
  int cleared_lines_count = clear_lines(game);

  if (cleared_lines_count > 0) {
    add_score(&game->info, cleared_lines_count);
    update_level(&game->info);
  }
}

/**
 * @brief Начисляет очки за очищенные линии и обновляет рекорд.
 *
 * @param info Указатель на структуру с информацией об игре.
 * @param cleared_lines Количество линий, очищенных за один раз.
 */
void add_score(GameInfo_t *info, int cleared_lines) {
  switch (cleared_lines) {
    case 1:
      info->score += 100;
      break;
    case 2:
      info->score += 300;
      break;
    case 3:
      info->score += 700;
      break;
    case 4:
      info->score += 1500;
      break;
  }

  if (info->score > info->high_score) {
    info->high_score = info->score;
  }
}

/**
 * @brief Обновляет уровень игрока на основе текущего счета.
 *
 * @param info Указатель на структуру с информацией об игре (счет, уровень).
 */
void update_level(GameInfo_t *info) {
  int new_level = (info->score / PTS_TILL_LVLUP) + 1;

  info->level = new_level > MAX_LEVEL ? MAX_LEVEL : new_level;
}

/**
 * @brief Инициализирует начальное состояние игры.
 * * Очищает игровое поле, сбрасывает счет и уровень,
 * загружает рекорд, генерирует первую фигуру.
 * @param game Указатель на главную структуру данных игры.
 */
void initialize_game(GameData_t *game) {
  srand(time(NULL));
  memset(game->board, 0, sizeof(game->board));

  int high_score = load_high_score();
  game->info = (GameInfo_t){0, high_score, 1, 0, false};

  game->next_piece_index = generate_new_shape();
  game->state = Start;

  game->timer.ticker = 0;
  game->timer.speed_threshold = 20;
}

/**
 * @brief Преобразует код нажатой клавиши в игровое действие.
 *
 * Является "чистой" функцией, которая не меняет состояние игры, а только
 * сопоставляет код клавиши (например, KEY_UP) с действием перечисления
 * (например, ActionRotate).
 * @param key Код клавиши, полученный от ncurses (например, getch()).
 * @return Соответствующее игровое действие типа UserAction_t.
 */
UserAction_t get_user_action(int key) {
  if (key == ERR) return ActionNone;
  if (key == 'q' || key == 'Q') return ActionTerminate;
  if (key == 'p' || key == 'P') return ActionPause;
  if (key == KEY_ENTER || key == '\n') return ActionStart;
  if (key == KEY_LEFT) return ActionMoveLeft;
  if (key == KEY_RIGHT) return ActionMoveRight;
  if (key == KEY_DOWN) return ActionMoveDown;
  if (key == KEY_UP) return ActionRotate;
  return ActionNone;
}

/**
 * @brief Применяет действие пользователя к состоянию игры.
 *
 * Эта функция-диспетчер принимает действие (например, ActionMoveLeft) и
 * вызывает соответствующие функции для изменения состояния игры.
 * @param game Указатель на главную структуру данных игры.
 * @param action Действие, полученное от get_user_action().
 */
void apply_user_action(GameData_t *game, UserAction_t action) {
  if (action == ActionTerminate) {
    game->state = GameOver;
    return;
  }
  if (action == ActionPause) {
    if (game->state != Start && game->state != GameOver) {
      game->info.pause = !game->info.pause;
    }
    return;
  }
  if (game->info.pause) return;

  if (game->state == Start && action == ActionStart) {
    game->state = Spawn;
    return;
  }

  if (game->state == Moving) {
    CurrentPiece_t temp = game->current_piece;
    if (action == ActionMoveLeft)
      move_piece(game, -1, 0);
    else if (action == ActionMoveRight)
      move_piece(game, 1, 0);
    else if (action == ActionRotate) {
      rotate_piece(game);
      if (check_collision(game)) {
        game->current_piece = temp;
      }
    } else if (action == ActionMoveDown) {
      while (!check_collision(game)) {
        game->current_piece.y++;
      }
      game->current_piece.y--;
      game->state = Attaching;
    }
  }
}

/**
 * @brief Обновляет состояние игры на основе таймера и текущего состояния.
 *
 * Эта функция является "сердцем" конечного автомата. Она отвечает за
 * автоматические переходы состояний, такие как падение фигуры по таймеру
 * (Shifting) или ее "прилипание" к полю (Attaching).
 * @param game Указатель на главную структуру данных игры.
 */
void update_game_state(GameData_t *game) {
  if (game->info.pause) return;

  if (game->state == Moving) {
    if (game->timer.ticker > game->timer.speed_threshold - game->info.level) {
      game->state = Shifting;
      game->timer.ticker = 0;
    }
    game->timer.ticker++;
  }

  switch (game->state) {
    case Spawn:
      if (spawn_new_piece(game)) {
        game->state = Moving;
      } else {
        game->state = GameOver;
      }
      break;

    case Shifting: {
      CurrentPiece_t temp = game->current_piece;
      move_piece(game, 0, 1);

      if (temp.y == game->current_piece.y) {
        game->state = Attaching;
      } else {
        game->state = Moving;
      }
      break;
    }

    case Attaching:
      imprint_piece_to_board(game);
      process_scoring_and_levelup(game);
      game->state = Spawn;
      break;

    case Start:
    case Moving:
    case GameOver:
      break;
  }
}

/**
 * @brief Выполняет вращение текущей фигуры на 90 градусов по часовой стрелке.
 *
 * Модифицирует матрицу `shape` в `game->current_piece`.
 * @param game Указатель на главную структуру данных игры.
 */
void rotate_piece(GameData_t *game) {
  int temp_shape[4][4] = {0};
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      temp_shape[i][j] = game->current_piece.shape[j][i];
    }
  }
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4 / 2; j++) {
      int temp = temp_shape[i][j];
      temp_shape[i][j] = temp_shape[i][3 - j];
      temp_shape[i][3 - j] = temp;
    }
  }
  memcpy(game->current_piece.shape, temp_shape, sizeof(int) * 16);
}

/**
 * @brief Загружает рекорд из файла "highscore.txt".
 *
 * @return int Загруженное значение рекорда, или 0, если файл не найден.
 */
int load_high_score() {
  FILE *file = fopen("highscore.txt", "r");
  if (file == NULL) {
    return 0;
  }
  int score = 0;
  fscanf(file, "%d", &score);
  fclose(file);
  return score;
}

/**
 * @brief Сохраняет текущее значение рекорда в файл "highscore.txt".
 *
 * @param score Значение рекорда для сохранения.
 */
void save_high_score(int score) {
  FILE *file = fopen("highscore.txt", "w");
  if (file == NULL) {
    return;
  }
  fprintf(file, "%d", score);
  fclose(file);
}