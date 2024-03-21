#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include "libtypeap.h"
#include "main.h"
#include "ring_buffer.h"

void cleanup(void) {
	configure_terminal(true);
	// show cursor
	printf("\033[?25h");
}

void handle_exit_signals(int signum) {
	(void)signum;
	exit(1);
}

int main() {

	struct sigaction act;
	act.sa_handler = handle_exit_signals;
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGKILL, &act, NULL);

	atexit(cleanup);

	struct winsize terminal_size;
    int out_code = ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal_size);

	if (out_code != 0) {
		typeap_print_error(__FILE__, __LINE__, NULL);
		return 1;
	}

	if (terminal_size.ws_col < MIN_TERMINAL_COLS || terminal_size.ws_row < MIN_TERMINAL_ROWS) {
		printf("Terminal to small.\n");
		printf("Needed: (%dx%d)\n", MIN_TERMINAL_COLS, MIN_TERMINAL_ROWS);
		printf("Current: (%dx%d)\n", terminal_size.ws_col, terminal_size.ws_row);
		return 1;
	}

	configure_terminal(false);

	// drop to even
	const int terminal_cols = terminal_size.ws_col & (~1);
	const int terminal_rows = terminal_size.ws_row;

	srand(time(NULL));

	RingBuffer *snake = ring_buffer_init((terminal_cols * terminal_rows) / 2);

	Coord curr_coord = {
		(terminal_cols / 4) & (~1) - 2,
		terminal_rows / 2
	};

	for (int i = 0; i < SNAKE_INIT_LENGHT; ++i) {
		curr_coord.x += 2;
		ring_buffer_enqueue(&snake, curr_coord);
	}

	Coord curr_fruit = gen_fruit(snake, terminal_cols, terminal_rows);

	// hide cursor
	printf("\033[?25l");

	draw(snake, curr_fruit, terminal_rows, terminal_cols);

	enum PlayerInput curr_direction = PI_DIRECTION_RIGHT;

	const Coord directions[4] = {
		{0, -1},
		{2, 0},
		{0, 1},
		{-2, 0}
	};

	struct timespec frames_deltatime = {
		.tv_sec = 0,
		.tv_nsec = 75 * 1000 * 1000
	};

	while (true) {
		enum PlayerInput player_input = get_player_input();

		if ((player_input == PI_DIRECTION_UP ||
			player_input == PI_DIRECTION_RIGHT ||
			player_input == PI_DIRECTION_DOWN ||
			player_input == PI_DIRECTION_LEFT) &&
			player_input + 2 != curr_direction &&
			player_input - 2 != curr_direction) {

			curr_direction = player_input;
		}

		curr_coord.x += directions[curr_direction].x;
		curr_coord.y += directions[curr_direction].y;

		if (curr_coord.x == 0 ||
			curr_coord.x == terminal_cols ||
			curr_coord.y == 3 ||
			curr_coord.y == terminal_rows) {

			break;
		}

		bool did_hit_itself = false;

		for (int i = 0; i < snake->lenght; ++i) {
			int snake_segment_index = (snake->read_index + i) % snake->size;
			if (snake->data[snake_segment_index].x == curr_coord.x &&
				snake->data[snake_segment_index].y == curr_coord.y) {

				did_hit_itself = true;
				break;
			}
		}

		if (did_hit_itself) {
			break;
		}

		ring_buffer_enqueue(&snake, curr_coord);

		if (curr_coord.x != curr_fruit.x ||
			curr_coord.y != curr_fruit.y) {

			ring_buffer_dequeue(snake, NULL);
		} else {

			curr_fruit = gen_fruit(snake, terminal_cols, terminal_rows);
		}

		draw(snake, curr_fruit, terminal_rows, terminal_cols);

		nanosleep(&frames_deltatime, NULL);
	}

	// go to top left
	printf("\033[H");
	// clean
	printf("\033[2J");

	printf("\n");
	printf("  Score: %d\n", snake->lenght - SNAKE_INIT_LENGHT);
	printf("\n");

	return 0;
}

Coord gen_fruit(RingBuffer *snake, int cols, int rows) {

	bool is_valid = false;
	Coord fruit = {-1, -1};

	while (!is_valid) {
		// subtract walls and score section (boxes = 2 cols x 1 row)
		// cols: 2 to (cols - 1)
		// rows: 4 to (rows - 1)
		fruit.x = (random() % (cols - 2) + 2) & (~1);
		fruit.y = random() % (rows - 4) + 4;

		bool does_overlap_with_snake = false;

		for (int i = 0; i < snake->lenght; ++i) {
			int snake_segment_index = (snake->read_index + i) % snake->size;
			if (fruit.x == snake->data[snake_segment_index].x ||
				fruit.y == snake->data[snake_segment_index].y) {

				does_overlap_with_snake = true;
				break;
			}
		}

		if (!does_overlap_with_snake) {
			is_valid = true;
		}
	};

	return fruit;
}

enum PlayerInput get_player_input() {

	enum PlayerInput out = PI_INVALID;

	char curr_char = getchar();

	while (curr_char != EOF) {
		if (curr_char == 'h' || curr_char == 'a') {
			out = PI_DIRECTION_LEFT;

		} else if (curr_char == 'j' || curr_char == 's') {
			out = PI_DIRECTION_DOWN;

		} else if (curr_char == 'k' || curr_char == 'w') {
			out = PI_DIRECTION_UP;

		} else if (curr_char == 'l' || curr_char == 'd') {
			out = PI_DIRECTION_RIGHT;

		} else if (curr_char == '\033' && getchar() == '[') { // Arrow keys
			switch (getchar()) {
				case 'A':
					out = PI_DIRECTION_UP;
					break;

				case 'B':
					out = PI_DIRECTION_DOWN;
					break;

				case 'C':
					out = PI_DIRECTION_RIGHT;
					break;

				case 'D':
					out = PI_DIRECTION_LEFT;
					break;

				default:
					break;
			}
		}

		if (out != PI_INVALID) {
			break;
		}

		curr_char = getchar();
	}

	while (getchar() != EOF);

	return out;
}

void draw(RingBuffer *snake, Coord fruit, int rows, int cols) {

	// clean screen
	printf("\033[2J");

	//// walls ////

	// go row col
	printf("\033[1;1H");
	printf(WALL_TOP_LEFT);

	for (int i = 2; i < cols; ++i) {
		// go row col
		printf("\033[1;%dH", i);
		printf(WALL_HORIZONTAL);
	}

	// go row col
	printf("\033[1;%dH", cols);
	printf(WALL_TOP_RIGHT);

	// go row col
	printf("\033[2;1H");
	printf(WALL_VERTICAL);

	// go row col
	printf("\033[2;%dH", cols);
	printf(WALL_VERTICAL);

	// go row col
	printf("\033[3;1H");
	printf(WALL_VERTICAL_T_LEFT);

	// go row col
	printf("\033[3;%dH", cols);
	printf(WALL_VERTICAL_T_RIGHT);

	for (int i = 2; i < cols; ++i) {
		// go row col
		printf("\033[3;%dH", i);
		printf(WALL_HORIZONTAL);
	}

	for (int i = 4; i < rows; ++i) {
		// go row col
		printf("\033[%d;1H", i);
		printf(WALL_VERTICAL);

		// go row col
		printf("\033[%d;%dH", i, cols);
		printf(WALL_VERTICAL);
	}

	// go row col
	printf("\033[%d;1H", rows);
	printf(WALL_BOTTOM_LEFT);

	// go row col
	printf("\033[%d;%dH", rows, cols);
	printf(WALL_BOTTOM_RIGHT);

	for (int i = 2; i < cols; ++i) {
		// go row col
		printf("\033[%d;%dH", rows, i);
		printf(WALL_HORIZONTAL);
	}

	// go row col
	printf("\033[2;3H");
	printf("Score: %d", snake->lenght - SNAKE_INIT_LENGHT);

	/* //// fruit //// */

	// go row col
	printf("\033[%d;%dH", fruit.y, fruit.x);
	printf(FRUIT_BODY_LEFT);
	printf(FRUIT_BODY_RIGHT);

	//// snake ////

	for (int i = 0; i < snake->lenght; ++i) {
		int snake_segment_index = (snake->read_index + i) % snake->size;
		// go row col
		printf("\033[%d;%dH", snake->data[snake_segment_index].y, snake->data[snake_segment_index].x);
		printf(SNAKE_BODY);
		printf(SNAKE_BODY);
	}

	fflush(stdout);
}

int configure_terminal(bool restore_default) {

	static bool is_configured = false;
	static struct termios old_terminal_config, new_terminal_config;
	static int old_stdin_flags;

	int out_code;

	if (!is_configured) {
		out_code = tcgetattr(STDIN_FILENO, &old_terminal_config);
		if (out_code != 0) {
			typeap_print_error(__FILE__, __LINE__, NULL);
			return out_code;
		}

		new_terminal_config = old_terminal_config;

		new_terminal_config.c_lflag &= ~(ICANON | ECHO);

		out_code = tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal_config);
		if (out_code != 0) {
			typeap_print_error(__FILE__, __LINE__, NULL);
			return out_code;
		}

		old_stdin_flags = fcntl(STDIN_FILENO, F_GETFL, 0);

		out_code = fcntl(STDIN_FILENO, F_SETFL, old_stdin_flags | O_NONBLOCK);
		if (out_code != 0) {
			typeap_print_error(__FILE__, __LINE__, NULL);
			return out_code;
		}

		is_configured = true;
	}

	if (restore_default && is_configured) {
		out_code = tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal_config);
		if (out_code != 0) {
			typeap_print_error(__FILE__, __LINE__, NULL);
			return out_code;
		}

		out_code = fcntl(STDIN_FILENO, F_SETFL, old_stdin_flags);
		if (out_code != 0) {
			typeap_print_error(__FILE__, __LINE__, NULL);
			return out_code;
		}

		is_configured = false;
	}

	return 0;
}
