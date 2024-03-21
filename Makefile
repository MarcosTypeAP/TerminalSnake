BIN_DIR = bin
SRC_DIRS = src lib
OBJ_DIR = obj
INCLUDE_DIRS = include

BIN = snake
BIN_PATH = $(BIN_DIR)/$(BIN)

CC = clang
# C_LIBS = -lm
C_FLAGS = -Wall -Wextra -Werror -std=c99 $(foreach DIR, $(INCLUDE_DIRS), -I$(DIR))

SRC_FILES = $(foreach DIR, $(SRC_DIRS), $(wildcard $(DIR)/*.c))
H_FILES = $(foreach DIR, $(SRC_DIRS) $(INCLUDE_DIRS), $(wildcard $(DIR)/*.h))

OBJ_FILES = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

NEEDED_DIRS = $(SRC_DIRS) $(OBJ_DIR) $(BIN_DIR) $(INCLUDE_DIRS) $(foreach DIR, $(SRC_DIRS), $(OBJ_DIR)/$(DIR))


all: $(BIN_PATH)

run: $(BIN_PATH)
	./$(BIN_PATH)

$(BIN_PATH): $(NEEDED_DIRS) $(OBJ_FILES) $(H_FILES)
	$(CC) $(C_FLAGS) $(C_LIBS) -o $(BIN_PATH) $(OBJ_FILES)
	chmod +x $(BIN_PATH)

$(OBJ_DIR)/%.o: %.c $(H_FILES)
	$(CC) $(C_FLAGS) $(C_LIBS) -c -o $@ $<

$(foreach DIR, $(NEEDED_DIRS), $(DIR)):
	mkdir -p $@

clean:
	rm -fr $(OBJ_DIR) $(BIN_DIR)
