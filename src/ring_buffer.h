#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdbool.h>

typedef struct {
	int x;
	int y;
} Coord;

typedef struct {
	int size;
	int lenght;
	int write_index;
	int read_index;
	Coord data[];
} RingBuffer;

RingBuffer *ring_buffer_init(int min_size);
bool ring_buffer_enqueue(RingBuffer **ring_buffer, Coord value);
bool ring_buffer_dequeue(RingBuffer *ring_buffer, Coord *value);
void ring_buffer_print(RingBuffer *ring_buffer);
RingBuffer *_ring_buffer_resize(RingBuffer *ring_buffer);

#endif
