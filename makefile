# === Makefile ===

CC := gcc
CFLAGS := -Iinclude
CFLAGS_DEBUG := -Wall -Wextra -Iinclude

SRC_DIR := src
OBJ_DIR := build
BIN_DIR := bin

# dirs
$(shell mkdir -p $(OBJ_DIR) $(BIN_DIR))

# programs
PROGRAMS := notify clock sudo battery_tray battery

# default
.PHONY: all clean
all: $(PROGRAMS)

# === Individual targets ===

notify: $(OBJ_DIR)/notify.o $(OBJ_DIR)/window.o
	$(CC) $^ -o $(BIN_DIR)/notify.exe -lgdi32

battery: $(OBJ_DIR)/battery.o $(OBJ_DIR)/window.o
	$(CC) $^ -o $(BIN_DIR)/battery.exe -lgdi32

clock: $(OBJ_DIR)/clock.o $(OBJ_DIR)/window.o
	$(CC) $^ -o $(BIN_DIR)/clock.exe -lgdi32

sudo: $(OBJ_DIR)/sudo.o
	$(CC) $^ -o $(BIN_DIR)/sudo.exe

battery_tray: $(OBJ_DIR)/battery_tray.o $(OBJ_DIR)/window.o
	$(CC) $^ -o $(BIN_DIR)/battery_tray.exe -luser32 -lgdi32 -lshell32 -mwindows

# === Generic build rule ===

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# === Clean ===
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
