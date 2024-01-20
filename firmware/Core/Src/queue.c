/*
 * queue.c
 *
 *  Created on: Mar 2, 2023
 *      Author: andrew
 */

#include <string.h>
#include <stdlib.h>

#include "queue.h"

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/** create an array of fixed length buffers */
queue_t * queue_create(int32_t queue_size, int32_t buffer_size)
{
	queue_t *queue = malloc(sizeof(queue_t));
	assert(queue!=NULL);
	memset(queue,0, sizeof(queue_t));
	queue->buffers = malloc(queue_size * buffer_size);
	assert(queue->buffers!=NULL);
	memset(queue->buffers, 0, queue_size * buffer_size);

	queue->head = 0;
	queue->tail = 0;
	queue->queue_size = queue_size;
	queue->buffer_size = buffer_size;
	return queue;
}

/** copy given buffer int32_to free buffer, advance head index */
void queue_put(queue_t * queue, uint8_t * buffer, int32_t buffer_size)
{
	int32_t elements_to_copy = MIN(buffer_size, queue->buffer_size);

	memset(queue->buffers+queue->head*queue->buffer_size,0,queue->buffer_size);
	memcpy(queue->buffers+queue->head*queue->buffer_size, buffer, elements_to_copy);

	if(++queue->head >= queue->queue_size)
	{
		queue->head = 0;
	}
}

/** copy tail buffer int32_to given buffer and advance tail index */
void queue_get(queue_t * queue, uint8_t * buffer, int32_t buffer_size)
{
	int32_t elements_to_copy = MIN(buffer_size, queue->buffer_size);

	memcpy(buffer, queue->buffers+queue->tail*queue->buffer_size,  elements_to_copy);

	if(++queue->tail >= queue->queue_size)
	{
		queue->tail = 0;
	}
}

/** return number if buffers in queue */
int32_t queue_size(queue_t * queue)
{
	return abs(queue->head - queue->tail);
}

/** free up memory */
void queue_delete(queue_t * queue)
{
	free(queue->buffers);
	free(queue);
}
