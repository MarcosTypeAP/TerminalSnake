#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "libtypeap.h"

extern int errno;

void typeap_print_error(char *filename, int line, char *message) {
#ifdef __LIBTYPEAP_DEBUG__
	fprintf(
		stderr,
		"Error in file \"%s\", near line %d: %s\n",
		filename,
		line,
		message == NULL ? strerror(errno) : message
	);
#else
	(void)message;
	(void)filename;
	(void)line;
#endif
}

size_t get_next_power_of_two(size_t value) {
	size_t result = 1;

	while (result < value) {
		result <<= 1;
	}

	return result;
}

void *typeap_malloc_aligned_size(size_t size) {
	return malloc(get_next_power_of_two(size));
}

void *typeap_realloc_aligned_size(void *pointer, size_t size) {
	return realloc(pointer, get_next_power_of_two(size));
}
