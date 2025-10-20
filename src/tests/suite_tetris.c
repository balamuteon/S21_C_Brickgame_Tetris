#include <check.h>
#include <stdio.h>  // Для работы с файлами в тестах

#include "brickgame/tetris/tetris.h"  // Подключаем нашу логику

// --- Тесты для функции generate_new_shape ---

START_TEST(test_generate_new_shape_range) {
  for (int i = 0; i < 1000; i++) {
    int shape_index = generate_new_shape();

    ck_assert_int_ge(shape_index, 0);
    ck_assert_int_le(shape_index, 6);
  }
}
END_TEST

//----------------------------------------------------------------------------
// утилиты для тестов

static void setup_game_with_piece(GameData_t *game, int piece_index) {
  initialize_game(game);
  memcpy(game->current_piece.shape, FIGURES[piece_index], sizeof(int) * 16);
  game->current_piece.color_index = piece_index + 1;
}

// --- Утилита для тестов: создает структуру GameInfo_t ---
static GameInfo_t create_game_info(int score, int level) {
  GameInfo_t info = {0};  // Инициализируем нулями
  info.score = score;
  info.level = level;
  return info;
}

//----------------------------------------------------------------------------
// --- Тесты для функции check_collision ---

START_TEST(test_collision_none) {
  GameData_t game;
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  // Помещаем фигуру в центр пустого поля
  game.current_piece.x = 3;
  game.current_piece.y = 5;

  bool result = check_collision(&game);
  // Ожидаем, что столкновений нет
  ck_assert_int_eq(result, false);
}
END_TEST

START_TEST(test_collision_with_left_wall) {
  GameData_t game;
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  // Фигура 'I' имеет пустой столбец слева, поэтому для коллизии x должен быть
  // -1
  game.current_piece.x = -1;
  game.current_piece.y = 5;

  bool result = check_collision(&game);
  ck_assert_int_eq(result, true);
}
END_TEST

START_TEST(test_collision_with_right_wall) {
  GameData_t game;
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  // Ставим фигуру так, чтобы ее правый край вышел за границу
  game.current_piece.x = BOARD_WIDTH - 3;  // Правый блок будет на x = 10
  game.current_piece.y = 5;

  bool result = check_collision(&game);
  ck_assert_int_eq(result, true);
}
END_TEST

START_TEST(test_collision_with_floor) {
  GameData_t game;
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  // Фигура 'I' лежит на 3й строке (индекс 2).
  // y = 18 -> блоки на y = 20, это коллизия
  game.current_piece.y = BOARD_HEIGHT - 2;
  game.current_piece.x = 3;

  bool result = check_collision(&game);
  ck_assert_int_eq(result, true);
}
END_TEST

START_TEST(test_collision_with_another_piece) {
  GameData_t game;
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  // Помещаем "стену" на поле
  game.board[10][5] = 1;

  // Двигаем нашу фигуру прямо на эту "стену"
  game.current_piece.x = 2;
  game.current_piece.y = 8;  // Блок фигуры окажется на board[10][5]

  bool result = check_collision(&game);
  ck_assert_int_eq(result, true);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции imprint_piece_to_board ---

START_TEST(test_imprint_standard) {
  GameData_t game;
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  // Помещаем фигуру 'I' на 10-ю строку
  game.current_piece.x = 3;
  game.current_piece.y = 8;  // Блоки фигуры будут на y = 8 + 2 = 10

  imprint_piece_to_board(&game);

  // Проверяем, что на доске появились блоки с правильным цветом (1)
  ck_assert_int_ne(game.board[10][3], 0);
  ck_assert_int_ne(game.board[10][4], 0);
  ck_assert_int_ne(game.board[10][5], 0);
  ck_assert_int_ne(game.board[10][6], 0);

  // Проверяем, что соседние ячейки остались пустыми
  ck_assert_int_eq(game.board[10][2], 0);
  ck_assert_int_eq(game.board[11][3], 0);
}
END_TEST

START_TEST(test_imprint_clipped_top) {
  GameData_t game;
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  // Вращаем палку, чтобы она стала вертикальной
  rotate_piece(&game);

  // Помещаем ее так, чтобы два верхних блока были "за кадром"
  game.current_piece.x = 5;
  game.current_piece.y = -2;  // Блоки будут на y = -2, -1, 0, 1

  imprint_piece_to_board(&game);

  // Проверяем, что впечатались только два нижних блока
  ck_assert_int_ne(game.board[0][6],
                   0);  // Блок на y=0 (в матрице фигуры y=-2+2)
  ck_assert_int_ne(game.board[1][6],
                   0);  // Блок на y=1 (в матрице фигуры y=-2+3)

  // Проверяем, что выше ничего нет
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      if (y > 1 && x == 6) {
        ck_assert_int_eq(game.board[y][x], 0);
      }
    }
  }
}
END_TEST
//----------------------------------------------------------------------------
// --- Тесты для функции clear_lines ---

START_TEST(test_clear_no_lines) {
  GameData_t game;
  initialize_game(&game);
  // Частично заполняем поле, но без полных линий
  game.board[19][5] = 1;
  game.board[18][2] = 2;

  int cleared = clear_lines(&game);

  // Ожидаем 0 очищенных линий
  ck_assert_int_eq(cleared, 0);
  // Убедимся, что поле не изменилось
  ck_assert_int_eq(game.board[19][5], 1);
}
END_TEST

START_TEST(test_clear_one_bottom_line) {
  GameData_t game;
  initialize_game(&game);
  // Заполняем нижнюю линию
  for (int x = 0; x < BOARD_WIDTH; x++) {
    game.board[19][x] = 1;
  }
  // Добавляем блок сверху, который должен сдвинуться вниз
  game.board[18][5] = 2;

  int cleared = clear_lines(&game);

  // Ожидаем 1 очищенную линию
  ck_assert_int_eq(cleared, 1);
  // Проверяем, что блок сверху сдвинулся на место очищенной линии
  ck_assert_int_eq(game.board[19][5], 2);
  // Проверяем, что его старое место теперь пусто
  ck_assert_int_eq(game.board[18][5], 0);
}
END_TEST

START_TEST(test_clear_four_lines_tetris) {
  GameData_t game;
  initialize_game(&game);
  // Заполняем 4 нижние линии
  for (int y = 16; y < 20; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      game.board[y][x] = y - 15;  // Разные цвета для наглядности
    }
  }

  int cleared = clear_lines(&game);

  // Ожидаем 4 очищенные линии
  ck_assert_int_eq(cleared, 4);

  // Проверяем, что все поле теперь пустое
  int sum = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      sum += game.board[y][x];
    }
  }
  ck_assert_int_eq(sum, 0);
}
END_TEST

START_TEST(test_clear_line_in_the_middle) {
  GameData_t game;
  initialize_game(&game);

  // Блок ниже очищаемой линии (должен остаться на месте)
  game.board[15][5] = 1;
  // Линия для очистки
  for (int x = 0; x < BOARD_WIDTH; x++) {
    game.board[14][x] = 2;
  }
  // Блок выше очищаемой линии (должен сдвинуться)
  game.board[13][5] = 3;

  int cleared = clear_lines(&game);

  ck_assert_int_eq(cleared, 1);
  // Проверяем, что нижний блок не сдвинулся
  ck_assert_int_eq(game.board[15][5], 1);
  // Проверяем, что верхний блок сдвинулся на место очищенной линии
  ck_assert_int_eq(game.board[14][5], 3);
  // Проверяем, что старое место верхнего блока пусто
  ck_assert_int_eq(game.board[13][5], 0);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции process_scoring_and_levelup ---

START_TEST(test_process_scoring_when_no_lines_cleared) {
  GameData_t game;
  initialize_game(&game);
  // Поле с "мусором", но без полных линий
  game.board[19][0] = 1;
  game.board[18][9] = 2;

  int initial_score = game.info.score;
  int initial_level = game.info.level;

  process_scoring_and_levelup(&game);

  // Проверяем, что score и level не изменились,
  // так как блок if не должен был выполниться
  ck_assert_int_eq(game.info.score, initial_score);
  ck_assert_int_eq(game.info.level, initial_level);
}
END_TEST

START_TEST(test_process_scoring_when_one_line_cleared) {
  GameData_t game;
  initialize_game(&game);
  // Заполняем одну линию
  for (int x = 0; x < BOARD_WIDTH; x++) {
    game.board[19][x] = 1;
  }

  process_scoring_and_levelup(&game);

  // Проверяем, что score изменился,
  // подтверждая, что add_score была вызвана.
  ck_assert_int_eq(game.info.score, 100);
  // Уровень не должен измениться
  ck_assert_int_eq(game.info.level, 1);
}
END_TEST

START_TEST(test_process_scoring_with_tetris_and_levelup) {
  GameData_t game;
  initialize_game(&game);
  // Устанавливаем счет близкий к повышению уровня
  game.info.score = 500;

  // Заполняем 4 линии для "Тетриса"
  for (int y = 16; y < 20; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      game.board[y][x] = 1;
    }
  }

  process_scoring_and_levelup(&game);

  // Ожидаемый счет: 500 (начальный) + 1500 (за тетрис) = 2000
  ck_assert_int_eq(game.info.score, 2000);

  // Ожидаемый уровень: 2000 / 600 = 3. Начальный 1 + 3 = 4
  ck_assert_int_eq(game.info.level, 4);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции update_level ---

START_TEST(test_level_initial_state) {
  GameInfo_t info = create_game_info(0, 1);

  update_level(&info);

  // При счете 0, уровень должен оставаться 1
  ck_assert_int_eq(info.level, 1);
}
END_TEST

START_TEST(test_level_just_before_levelup) {
  // 599 очков / 600 = 0. +1 = 1.
  GameInfo_t info = create_game_info(599, 1);

  update_level(&info);

  // Уровень не должен повыситься
  ck_assert_int_eq(info.level, 1);
}
END_TEST

START_TEST(test_level_exact_levelup) {
  // 600 очков / 600 = 1. +1 = 2.
  GameInfo_t info = create_game_info(600, 1);

  update_level(&info);

  // Уровень должен стать 2
  ck_assert_int_eq(info.level, 2);
}
END_TEST

START_TEST(test_level_multiple_levelup) {
  // 1800 очков / 600 = 3. +1 = 4.
  GameInfo_t info = create_game_info(1800, 1);

  update_level(&info);

  // Уровень должен стать 4
  ck_assert_int_eq(info.level, 4);
}
END_TEST

START_TEST(test_level_max_level_cap) {
  // 99999 / 600 = 166. +1 = 167. Это больше MAX_LEVEL (10)
  GameInfo_t info = create_game_info(99999, 9);

  update_level(&info);

  // Уровень должен быть ограничен максимальным значением
  ck_assert_int_eq(info.level, MAX_LEVEL);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции initialize_game ---

START_TEST(test_initialize_with_no_highscore_file) {
  // Убедимся, что файла нет перед тестом
  remove("highscore.txt");

  GameData_t game;
  initialize_game(&game);

  // Проверяем базовые состояния
  ck_assert_int_eq(game.state, Start);
  ck_assert_int_eq(game.info.score, 0);
  ck_assert_int_eq(game.info.level, 1);
  ck_assert(game.info.pause == false);

  ck_assert_int_ge(game.next_piece_index, 0);
  ck_assert_int_lt(game.next_piece_index, 7);

  // Если файла не было, рекорд должен быть 0
  ck_assert_int_eq(game.info.high_score, 0);

  // Проверяем, что доска пуста
  int board_sum = 0;
  for (int y = 0; y < BOARD_HEIGHT; y++) {
    for (int x = 0; x < BOARD_WIDTH; x++) {
      board_sum += game.board[y][x];
    }
  }
  ck_assert_int_eq(board_sum, 0);
}
END_TEST

START_TEST(test_initialize_with_existing_highscore) {
  // Создаем тестовый файл с рекордом
  FILE *file = fopen("highscore.txt", "w");
  fprintf(file, "9500");
  fclose(file);

  GameData_t game;
  initialize_game(&game);

  // Проверяем, что рекорд был успешно загружен
  ck_assert_int_eq(game.info.high_score, 9500);

  // Очищаем за собой
  remove("highscore.txt");
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции get_user_action ---

START_TEST(test_get_action_movement) {
  // Проверяем все клавиши движения
  ck_assert_int_eq(get_user_action(KEY_LEFT), ActionMoveLeft);
  ck_assert_int_eq(get_user_action(KEY_RIGHT), ActionMoveRight);
  ck_assert_int_eq(get_user_action(KEY_DOWN), ActionMoveDown);
  ck_assert_int_eq(get_user_action(KEY_UP), ActionRotate);
}
END_TEST

START_TEST(test_get_action_game_control) {
  // Проверяем клавиши управления игрой
  ck_assert_int_eq(get_user_action('q'), ActionTerminate);
  ck_assert_int_eq(get_user_action('Q'), ActionTerminate);
  ck_assert_int_eq(get_user_action('p'), ActionPause);
  ck_assert_int_eq(get_user_action('P'), ActionPause);
  ck_assert_int_eq(get_user_action(KEY_ENTER), ActionStart);
  ck_assert_int_eq(get_user_action('\n'), ActionStart);
}
END_TEST

START_TEST(test_get_action_special_cases) {
  // Проверяем случай отсутствия ввода
  ck_assert_int_eq(get_user_action(ERR), ActionNone);
  // Проверяем любую другую, не назначенную клавишу
  ck_assert_int_eq(get_user_action('a'), ActionNone);
  ck_assert_int_eq(get_user_action(' '), ActionNone);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции rotate_piece ---
// {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},  // L

START_TEST(test_rotate_L_piece_once) {
  GameData_t game;
  setup_game_with_piece(&game, 3);

  rotate_piece(&game);

  ck_assert_int_ne(game.current_piece.shape[0][1], 0);
  ck_assert_int_ne(game.current_piece.shape[1][1], 0);
  ck_assert_int_ne(game.current_piece.shape[2][1], 0);
  ck_assert_int_ne(game.current_piece.shape[2][2], 0);

  ck_assert_int_eq(game.current_piece.shape[0][0], 0);
  ck_assert_int_eq(game.current_piece.shape[0][2], 0);
  ck_assert_int_eq(game.current_piece.shape[2][0], 0);
  ck_assert_int_eq(game.current_piece.shape[2][3], 0);
}
END_TEST

START_TEST(test_rotate_I_piece_full_cycle) {
  GameData_t game;
  // Создаем игру с фигурой 'I' (индекс 0)
  int piece_index = 0;
  setup_game_with_piece(&game, piece_index);

  int initial_shape[4][4];
  memcpy(initial_shape, game.current_piece.shape, sizeof(int) * 16);

  rotate_piece(&game);
  rotate_piece(&game);
  rotate_piece(&game);
  rotate_piece(&game);

  int result =
      memcmp(initial_shape, game.current_piece.shape, sizeof(int) * 16);
  ck_assert_int_eq(result, 0);  // Ожидаем, что они идентичны (memcmp вернет 0)
}
END_TEST

START_TEST(test_rotate_O_piece_no_change) {
  GameData_t game;
  // Создаем игру с фигурой 'O'
  int piece_index = 1;
  setup_game_with_piece(&game, piece_index);

  int initial_shape[4][4];
  memcpy(initial_shape, game.current_piece.shape, sizeof(int) * 16);

  rotate_piece(&game);

  int result =
      memcmp(initial_shape, game.current_piece.shape, sizeof(int) * 16);
  ck_assert_int_eq(result, 0);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функций работы с файлами ---

START_TEST(test_load_highscore_no_file_exists) {
  // 1. Подготовка: убеждаемся, что файла нет
  remove("highscore.txt");

  // 2. Действие: вызываем функцию
  int high_score = load_high_score();

  // 3. Проверка: ожидаем 0, так как файла не было
  ck_assert_int_eq(high_score, 0);
}
END_TEST

START_TEST(test_save_and_load_highscore_cycle) {
  // 1. Подготовка: убеждаемся, что старого файла нет
  remove("highscore.txt");
  int expected_score = 8500;

  // 2. Действие 1: сохраняем новое значение рекорда
  save_high_score(expected_score);

  // 3. Действие 2: сразу же читаем его обратно
  int loaded_score = load_high_score();

  // 4. Проверка: убеждаемся, что прочитанное значение совпадает с сохраненным
  ck_assert_int_eq(loaded_score, expected_score);

  // 5. Очистка: удаляем созданный файл
  remove("highscore.txt");
}
END_TEST

START_TEST(test_save_overwrites_old_highscore) {
  // 1. Подготовка: создаем файл со старым рекордом
  save_high_score(1000);

  // 2. Действие: сохраняем новый, больший рекорд
  int new_expected_score = 5000;
  save_high_score(new_expected_score);

  // 3. Проверка: читаем и убеждаемся, что значение перезаписалось
  int loaded_score = load_high_score();
  ck_assert_int_eq(loaded_score, new_expected_score);

  // 4. Очистка
  remove("highscore.txt");
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции spawn_new_piece ---
START_TEST(test_spawn_success) {
  GameData_t game;
  initialize_game(&game);

  // Устанавливаем фигуру, которую ожидаем заспавнить ('I')
  game.next_piece_index = 0;
  // int old_next_piece_index = game.next_piece_index;

  bool result = spawn_new_piece(&game);

  // 1. Проверяем, что спавн прошел успешно
  ck_assert_int_eq(result, true);

  int mem_res = -1;
  int idx = 0;
  // 2. Проверяем, что фигура скопировалась в current_piece
  for (idx = 0; idx < 7; idx++) {
    mem_res = memcmp(game.current_piece.shape, FIGURES[idx], sizeof(int) * 16);
    if (mem_res == 0) {
      break;
    }
  }
  ck_assert_int_eq(mem_res, 0);

  // 3. Проверяем начальные координаты и цвет
  ck_assert_int_eq(game.current_piece.x, 3);  // (10 / 2) - 2
  ck_assert_int_eq(game.current_piece.y, -2);
  ck_assert_int_eq(game.current_piece.color_index, idx + 1);  // index + 1
}
END_TEST

START_TEST(test_spawn_fail_game_over) {
  GameData_t game;
  initialize_game(&game);
  game.next_piece_index = 0;  // Будем спавнить 'I'

  // Блокируем зону спавна
  game.board[0][4] = 1;

  bool result = spawn_new_piece(&game);

  // Ожидаем, что спавн провалится
  ck_assert_int_eq(result, false);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции move_piece ---

START_TEST(test_move_piece_success) {
  GameData_t game;
  initialize_game(&game);
  game.current_piece.x = 5;
  game.current_piece.y = 5;

  int initial_x = game.current_piece.x;

  // Двигаем вправо
  move_piece(&game, 1, 0);

  // Проверяем, что координата изменилась
  ck_assert_int_eq(game.current_piece.x, initial_x + 1);
}
END_TEST

START_TEST(test_move_piece_fail_wall) {
  GameData_t game;
  initialize_game(&game);
  // Ставим фигуру 'I' у левой стены (ее блоки начинаются с x=0)
  game.current_piece.x = 0;
  game.current_piece.y = 5;

  // Пытаемся сдвинуть влево
  move_piece(&game, -1, 0);

  // Проверяем, что координата НЕ изменилась
  ck_assert_int_eq(game.current_piece.x, 0);
}
END_TEST

START_TEST(test_move_piece_fail_block) {
  GameData_t game;
  initialize_game(&game);

  // Ставим блок на поле
  game.board[10][5] = 1;

  // Ставим нашу фигуру прямо над ним
  game.current_piece.x = 2;  // Блок фигуры будет на x = 2 + 3 = 5
  game.current_piece.y = 8;  // Блок фигуры будет на y = 8 + 2 = 10

  // Пытаемся сдвинуться вниз на этот блок
  move_piece(&game, 0, 1);

  // Проверяем, что координата y НЕ изменилась
  ck_assert_int_eq(game.current_piece.y, 8);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции update_game_state ---

START_TEST(test_update_state_spawn_to_moving) {
  GameData_t game;
  setup_game_with_piece(&game, 0);
  game.state = Spawn;

  update_game_state(&game);

  // Ожидаем, что спавн пройдет успешно и состояние изменится на Moving
  ck_assert_int_eq(game.state, Moving);
}
END_TEST

START_TEST(test_update_state_spawn_to_gameover) {
  GameData_t game;
  setup_game_with_piece(&game, 0);
  game.state = Spawn;
  // Блокируем зону спавна
  game.board[0][4] = 1;

  update_game_state(&game);

  // Ожидаем, что спавн провалится и игра закончится
  ck_assert_int_eq(game.state, GameOver);
}
END_TEST

START_TEST(test_update_state_shifting_to_attaching) {
  GameData_t game;
  setup_game_with_piece(&game, 0);
  game.state = Shifting;
  // Ставим фигуру прямо над полом
  game.current_piece.y = BOARD_HEIGHT - 3;

  update_game_state(&game);

  // Ожидаем, что после сдвига и коллизии состояние станет Attaching
  ck_assert_int_eq(game.state, Attaching);
}
END_TEST

START_TEST(test_update_state_attaching_to_spawn) {
  GameData_t game;
  setup_game_with_piece(&game, 0);  // Создаем игру с фигурой 'I'
  game.state = Attaching;

  // Вручную помещаем фигуру на доску, чтобы было что впечатывать
  game.current_piece.x = 3;
  game.current_piece.y = 10;

  update_game_state(&game);

  // Ожидаем, что после прикрепления мы готовы спавнить новую фигуру
  ck_assert_int_eq(game.state, Spawn);
  // Проверяем, что фигура 'I' действительно "впечаталась" на 12-й строке
  ck_assert_int_eq(game.board[12][3], 1);
}
END_TEST

START_TEST(test_update_state_timer_triggers_shifting) {
  GameData_t game;
  setup_game_with_piece(&game, 0);
  game.state = Moving;

  game.current_piece.x = 3;
  game.current_piece.y = 5;

  game.timer.ticker = game.timer.speed_threshold + 1;

  update_game_state(&game);

  ck_assert_int_eq(game.state, Moving);
  ck_assert_int_eq(game.current_piece.y, 6);
  ck_assert_int_eq(game.timer.ticker, 1);
}
END_TEST

//----------------------------------------------------------------------------
// --- Тесты для функции apply_user_action ---

START_TEST(test_action_start_game) {
  GameData_t game;
  initialize_game(&game);  // game.state == Start

  apply_user_action(&game, ActionStart);

  ck_assert_int_eq(game.state, Spawn);
}
END_TEST

START_TEST(test_action_pause_and_terminate) {
  GameData_t game;
  setup_game_with_piece(&game, 0);
  game.current_piece.x = 3;  // Безопасная позиция
  game.current_piece.y = 5;
  game.state = Moving;

  // Ставим паузу
  apply_user_action(&game, ActionPause);
  ck_assert_int_eq(game.info.pause, true);

  // Пытаемся двигаться на паузе - не должно работать
  int initial_x = game.current_piece.x;
  apply_user_action(&game, ActionMoveRight);
  ck_assert_int_eq(game.current_piece.x, initial_x);

  // Снимаем паузу
  apply_user_action(&game, ActionPause);
  ck_assert_int_eq(game.info.pause, false);

  // Завершаем игру
  apply_user_action(&game, ActionTerminate);
  ck_assert_int_eq(game.state, GameOver);
}
END_TEST

START_TEST(test_action_rotation_with_collision_cancel) {
  GameData_t game;
  setup_game_with_piece(&game, 0);
  game.current_piece.x = BOARD_WIDTH - 1;
  game.current_piece.y = 5;
  game.state = Moving;

  // Ставим фигуру у стены, чтобы вращение было невозможно

  // Сохраняем исходную форму
  int initial_shape[4][4];
  memcpy(initial_shape, game.current_piece.shape, sizeof(int) * 16);

  apply_user_action(&game, ActionRotate);

  // Проверяем, что форма не изменилась, так как вращение было отменено
  int result =
      memcmp(initial_shape, game.current_piece.shape, sizeof(int) * 16);
  ck_assert_int_eq(result, 0);
}
END_TEST

START_TEST(test_action_hard_drop) {
  GameData_t game;
  setup_game_with_piece(&game, 0);
  game.current_piece.x = 3;  // Безопасная позиция
  game.current_piece.y = 5;
  game.state = Moving;

  apply_user_action(&game, ActionMoveDown);

  // Проверяем, что состояние перешло в Attaching
  ck_assert_int_eq(game.state, Attaching);
  // Проверяем, что фигура упала на дно
  // Начальный y=5, блоки на y=7. Падение на y=17, блоки на y=19
  ck_assert_int_eq(game.current_piece.y, 17);
}
END_TEST

START_TEST(test_action_ignored_in_wrong_state) {
  GameData_t game;
  initialize_game(&game);  // game.state == Start

  // Пытаемся двигаться на стартовом экране
  apply_user_action(&game, ActionMoveLeft);

  // Состояние не должно измениться
  ck_assert_int_eq(game.state, Start);
}
END_TEST

//----------------------------------------------------------------------------
// Создание тестового набора для TetrisGame
Suite *tetris_suite_create(void) {
  Suite *s = suite_create("TetrisGame");
  /// --- Тесты генерации фигур ---
  TCase *tc_generation = tcase_create("Generation");
  tcase_add_test(tc_generation, test_generate_new_shape_range);
  suite_add_tcase(s, tc_generation);

  /// --- Тесты столкновений ---
  TCase *tc_collision = tcase_create("Collision");
  tcase_add_test(tc_collision, test_collision_none);
  tcase_add_test(tc_collision, test_collision_with_left_wall);
  tcase_add_test(tc_collision, test_collision_with_right_wall);
  tcase_add_test(tc_collision, test_collision_with_floor);
  tcase_add_test(tc_collision, test_collision_with_another_piece);
  suite_add_tcase(s, tc_collision);

  // --- Тесты прикрепления фигур на доску ---
  TCase *tc_imprint = tcase_create("Imprint");
  tcase_add_test(tc_imprint, test_imprint_standard);
  tcase_add_test(tc_imprint, test_imprint_clipped_top);
  suite_add_tcase(s, tc_imprint);

  // --- Тесты очистки линий ---
  TCase *tc_lines = tcase_create("Line Clearing");
  tcase_add_test(tc_lines, test_clear_no_lines);
  tcase_add_test(tc_lines, test_clear_one_bottom_line);
  tcase_add_test(tc_lines, test_clear_four_lines_tetris);
  tcase_add_test(tc_lines, test_clear_line_in_the_middle);
  suite_add_tcase(s, tc_lines);

  // --- Тесты обработки очков и повышения уровня ---
  TCase *tc_scoring = tcase_create("Scoring Orchestrator");
  tcase_add_test(tc_scoring, test_process_scoring_when_no_lines_cleared);
  tcase_add_test(tc_scoring, test_process_scoring_when_one_line_cleared);
  tcase_add_test(tc_scoring, test_process_scoring_with_tetris_and_levelup);
  suite_add_tcase(s, tc_scoring);

  // --- Тесты для функции update_level ---
  TCase *tc_leveling = tcase_create("Leveling Logic");
  tcase_add_test(tc_leveling, test_level_initial_state);
  tcase_add_test(tc_leveling, test_level_just_before_levelup);
  tcase_add_test(tc_leveling, test_level_exact_levelup);
  tcase_add_test(tc_leveling, test_level_multiple_levelup);
  tcase_add_test(tc_leveling, test_level_max_level_cap);
  suite_add_tcase(s, tc_leveling);

  // --- Тесты для функции initialize_game ---
  TCase *tc_init = tcase_create("Initialization");
  tcase_add_test(tc_init, test_initialize_with_no_highscore_file);
  tcase_add_test(tc_init, test_initialize_with_existing_highscore);
  suite_add_tcase(s, tc_init);

  // --- Тесты для функции get_user_action ---
  TCase *tc_actions = tcase_create("User Actions");
  tcase_add_test(tc_actions, test_get_action_movement);
  tcase_add_test(tc_actions, test_get_action_game_control);
  tcase_add_test(tc_actions, test_get_action_special_cases);
  suite_add_tcase(s, tc_actions);

  // --- Тесты для функции rotate_piece ---
  TCase *tc_rotation = tcase_create("Rotation");
  tcase_add_test(tc_rotation, test_rotate_L_piece_once);
  tcase_add_test(tc_rotation, test_rotate_I_piece_full_cycle);
  tcase_add_test(tc_rotation, test_rotate_O_piece_no_change);
  suite_add_tcase(s, tc_rotation);

  // --- Тесты для функций работы с файлами ---
  TCase *tc_file_io = tcase_create("File IO");
  tcase_add_test(tc_file_io, test_load_highscore_no_file_exists);
  tcase_add_test(tc_file_io, test_save_and_load_highscore_cycle);
  tcase_add_test(tc_file_io, test_save_overwrites_old_highscore);
  suite_add_tcase(s, tc_file_io);

  // --- Тесты для функции spawn_new_piece ---
  TCase *tc_spawn = tcase_create("Spawn Piece");
  tcase_add_test(tc_spawn, test_spawn_success);
  tcase_add_test(tc_spawn, test_spawn_fail_game_over);
  suite_add_tcase(s, tc_spawn);

  // --- Тесты для функции move_piece ---
  TCase *tc_move = tcase_create("Move Piece");
  tcase_add_test(tc_move, test_move_piece_success);
  tcase_add_test(tc_move, test_move_piece_fail_wall);
  tcase_add_test(tc_move, test_move_piece_fail_block);
  suite_add_tcase(s, tc_move);

  // --- Тесты для функции update_game_state ---
  TCase *tc_fsm = tcase_create("Finite State Machine");
  tcase_add_test(tc_fsm, test_update_state_spawn_to_moving);
  tcase_add_test(tc_fsm, test_update_state_spawn_to_gameover);
  tcase_add_test(tc_fsm, test_update_state_shifting_to_attaching);
  tcase_add_test(tc_fsm, test_update_state_attaching_to_spawn);
  tcase_add_test(tc_fsm, test_update_state_timer_triggers_shifting);
  suite_add_tcase(s, tc_fsm);

  TCase *tc_user_input = tcase_create("User Input Handling");
  tcase_add_test(tc_user_input, test_action_start_game);
  tcase_add_test(tc_user_input, test_action_pause_and_terminate);
  tcase_add_test(tc_user_input, test_action_rotation_with_collision_cancel);
  tcase_add_test(tc_user_input, test_action_hard_drop);
  tcase_add_test(tc_user_input, test_action_ignored_in_wrong_state);
  suite_add_tcase(s, tc_user_input);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s = tetris_suite_create();
  SRunner *sr = srunner_create(s);
  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? 0 : 1;
}