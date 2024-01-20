/* 
 * queue.h
 *
 *  Created on: Mar 2, 2023
 *      Author: andrew
 */

#ifndef QUEUE_H
#define	QUEUE_H

#include <stdio.h>
#include <stdint.h>

typedef struct
{
	uint8_t *buffers;
	int32_t queue_size;
	int32_t buffer_size;
	int32_t head;
	int32_t tail;
}
queue_t;

queue_t *queue_create(int32_t queue_size, int32_t buffer_size);
void queue_put(queue_t * queue, uint8_t * buffer, int32_t buffer_size);
void queue_get(queue_t * queue, uint8_t * buffer, int32_t buffer_size);
int32_t queue_size(queue_t * queue);
void queue_delete(queue_t * queue);

#endif	/* QUEUE_H */

