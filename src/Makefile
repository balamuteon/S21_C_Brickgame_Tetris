# ============================================================================
# Флаги компиляции и линковки
# ============================================================================
CFLAGS = -std=c11 -Wall -Wextra -Werror -I. -g
LDFLAGS = -lncursesw
GCOV_FLAGS = --coverage

# ============================================================================
# Переменные проекта
# ============================================================================
# --- Основное приложение ---
TARGET_NAME = tetris
BUILD_DIR = ../build
TARGET = $(BUILD_DIR)/$(TARGET_NAME)

# --- Статическая библиотека ---
LIB_NAME = tetris
LIBRARY = $(BUILD_DIR)/lib$(LIB_NAME).a
LIB_SRC = brickgame/tetris/tetris.c
LIB_OBJ = $(LIB_SRC:.c=.o)

# --- Исполняемая часть (без логики) ---
APP_SRC = gui/cli/view.c cmd/main.c
APP_OBJ = $(APP_SRC:.c=.o)

# --- Тесты ---
TEST_SRC = tests/suite_tetris.c
TEST_RUNNER = $(BUILD_DIR)/$(TARGET_NAME)_test
REPORT_DIR = report

.PHONY: all clean install uninstall dist dvi test gcov_report format leaks

# ============================================================================
# ОСНОВНЫЕ ЦЕЛИ СБОРКИ
# ============================================================================

all: $(TARGET)

$(TARGET): $(APP_OBJ) $(LIBRARY)
	@echo "Linking final executable: $(TARGET)"
	@mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) $(APP_OBJ) -o $@ -L$(BUILD_DIR) -l$(LIB_NAME) $(LDFLAGS)

$(LIBRARY): $(LIB_OBJ)
	@echo "Creating static library: $(LIBRARY)"
	@mkdir -p $(BUILD_DIR)
	ar rcs $@ $(LIB_OBJ)

%.o: %.c
	@echo "Compiling $< -> $@"
	gcc $(CFLAGS) -c $< -o $@

# ============================================================================
# СЛУЖЕБНЫЕ ЦЕЛИ
# ============================================================================

clean:
	@echo "Cleaning up project..."
	rm -rf $(BUILD_DIR) $(REPORT_DIR) doxygen
	find . -type f \( -name "*.o" -o -name "*.gcda" -o -name "*.gcno" -o -name "*.info" \) -delete

install: all
	@echo "Installing $(TARGET_NAME) to /usr/local/bin..."
	sudo mkdir -p /usr/local/bin
	sudo cp $(TARGET) /usr/local/bin/$(TARGET_NAME)

uninstall:
	@echo "Uninstalling $(TARGET_NAME) from /usr/local/bin..."
	sudo rm -f /usr/local/bin/$(TARGET_NAME)

dist: clean
	@echo "Creating source archive..."
	@mkdir -p $(BUILD_DIR)
	tar -czvf $(BUILD_DIR)/tetris-v1.0.tar.gz Makefile brickgame/ cmd/ gui/ tests/

dvi:
	@echo "Generating Doxygen documentation..."
	doxygen Doxyfile
	@echo "Documentation generated in 'src/doxygen/html/'. Opening..."
	xdg-open doxygen/html/index.html || open doxygen/html/index.html

# ============================================================================
# ЦЕЛИ ТЕСТИРОВАНИЯ, ПОКРЫТИЯ И АНАЛИЗА
# ============================================================================

test: $(LIBRARY)
	@echo "--- Running tests ---"
	@mkdir -p $(BUILD_DIR)
	gcc $(CFLAGS) $(TEST_SRC) -o $(TEST_RUNNER) -L$(BUILD_DIR) -l$(LIB_NAME) -lcheck
	./$(TEST_RUNNER)

gcov_report:
	@echo "--- Generating coverage report ---"
	gcc $(CFLAGS) $(GCOV_FLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)
	gcc $(CFLAGS) $(GCOV_FLAGS) -c $(TEST_SRC) -o $(TEST_SRC:.c=.o)
	gcc $(GCOV_FLAGS) $(TEST_SRC:.c=.o) $(LIB_OBJ) -o gcov_test_runner -lcheck
	./gcov_test_runner
	@mkdir -p $(REPORT_DIR)
	lcov -t "tetris_coverage" -o $(REPORT_DIR)/coverage.info -c -d .
	genhtml -o $(REPORT_DIR) $(REPORT_DIR)/coverage.info
	rm -f gcov_test_runner
	find . -type f \( -name "*.gcda" -o -name "*.gcno" -o -name "*.o" \) -delete
	@echo "Отчет о покрытии сгенерирован в папке '$(REPORT_DIR)'"
	xdg-open $(REPORT_DIR)/index.html || open $(REPORT_DIR)/index.html

leaks: all
	@echo "--- Running Valgrind Memcheck ---"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

format:
	@echo "Running clang-format..."
	cp ../materials/linters/.clang-format .
	find . -name "*.c" -o -name "*.h" | xargs clang-format -n
	@echo "Formatting check:"
	find . -name "*.c" -o -name "*.h" | xargs clang-format -i