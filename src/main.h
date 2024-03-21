#ifndef __MAIN_H__
#define __MAIN_H__

#include "ring_buffer.h"
#include <stdbool.h>

#define MIN_TERMINAL_ROWS 20
#define MIN_TERMINAL_COLS 40

#define WALL_VERTICAL "║"
#define WALL_HORIZONTAL "═"
#define WALL_TOP_LEFT "╔"
#define WALL_TOP_RIGHT "╗"
#define WALL_BOTTOM_LEFT "╚"
#define WALL_BOTTOM_RIGHT "╝"
#define WALL_VERTICAL_T_LEFT "╠"
#define WALL_VERTICAL_T_RIGHT "╣"

#define SNAKE_BODY "█"

#define SNAKE_INIT_LENGHT 4

#define FRUIT_BODY_LEFT "█"
#define FRUIT_BODY_RIGHT "█"

enum PlayerInput {
	PI_INVALID = -1,
	PI_DIRECTION_UP,
	PI_DIRECTION_RIGHT,
	PI_DIRECTION_DOWN,
	PI_DIRECTION_LEFT,
};

int configure_terminal(bool restore_default);

void draw(RingBuffer *snake, Coord fruit, int height, int width);

enum PlayerInput get_player_input();

Coord gen_fruit(RingBuffer *snake, int cols, int rows);

#endif
