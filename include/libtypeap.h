#ifndef __TYPEAP_H__
#define __TYPEAP_H__

#include <stddef.h>

#define __LIBTYPEAP_DEBUG__

void typeap_print_error(char *filename, int line, char *message);
size_t get_next_power_of_two(size_t value);
void *typeap_malloc_aligned_size(size_t size);
void *typeap_realloc_aligned_size(void *ptr, size_t size);

#endif
