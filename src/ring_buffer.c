#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "ring_buffer.h"
#include "libtypeap.h"

RingBuffer *ring_buffer_init(int min_size) {

	size_t elements_size = sizeof(*((RingBuffer *)NULL)->data);
	size_t size_to_allocate = get_next_power_of_two(sizeof(RingBuffer) + elements_size * min_size);

	RingBuffer *ring_buffer = malloc(size_to_allocate);

	if (ring_buffer == NULL) {
		typeap_print_error(__FILE__, __LINE__, NULL);
		return NULL;
	}

	ring_buffer->size = (size_to_allocate - sizeof(RingBuffer)) / elements_size;
	ring_buffer->lenght = 0;
	ring_buffer->write_index = ring_buffer->read_index = 0;

	return ring_buffer;
}

bool ring_buffer_enqueue(RingBuffer **ring_buffer, Coord value) {

	if ((*ring_buffer)->lenght == (*ring_buffer)->size) {
		*ring_buffer = _ring_buffer_resize(*ring_buffer);

		if (*ring_buffer == NULL) {
			typeap_print_error(__FILE__, __LINE__, "Failed to resize ring_buffer.");
			return false;
		}
	}

	RingBuffer *new_ring_buffer = *ring_buffer;

	new_ring_buffer->data[new_ring_buffer->write_index] = value;
	new_ring_buffer->write_index = (new_ring_buffer->write_index + 1) % new_ring_buffer->size;
	new_ring_buffer->lenght++;

	return true;
}

bool ring_buffer_dequeue(RingBuffer *ring_buffer, Coord *value) {

	if (ring_buffer->lenght == 0) {
		typeap_print_error(__FILE__, __LINE__, "Cannot dequeue, queue is empty.");
		return false;
	}

	if (value != NULL) {
		*value = ring_buffer->data[ring_buffer->read_index];
	}
	ring_buffer->read_index = (ring_buffer->read_index + 1) % ring_buffer->size;
	ring_buffer->lenght--;

	return true;
}

void ring_buffer_print(RingBuffer *ring_buffer) {

	for (int i = 0; i < ring_buffer->size; ++i) {
		printf("%d %d", ring_buffer->data[i].x, ring_buffer->data[i].y);
		if (i == ring_buffer->write_index) {
			printf("w ");
		}
		if (i == ring_buffer->read_index) {
			printf("r");
		}
		printf("\n");
	}
	printf("\n");
}

RingBuffer *_ring_buffer_resize(RingBuffer *ring_buffer) {

	size_t elements_size = sizeof(*((RingBuffer *)NULL)->data);
	size_t new_size_to_allocate = get_next_power_of_two(sizeof(RingBuffer) + (ring_buffer->size + 1) * elements_size);

	ring_buffer = realloc(ring_buffer, new_size_to_allocate);

	if (ring_buffer == NULL) {
		typeap_print_error(__FILE__, __LINE__, NULL);
		return NULL;
	}

	int new_buffer_size = (new_size_to_allocate - sizeof(RingBuffer)) / elements_size;

	for (int i = 0; i < ring_buffer->write_index; ++i) {
		ring_buffer->data[ring_buffer->size + i] = ring_buffer->data[i];
	}

	ring_buffer->write_index = ring_buffer->read_index + ring_buffer->lenght;
	ring_buffer->size = new_buffer_size;

	return ring_buffer;
}
