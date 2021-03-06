/*
 * syscalls.c
 *
 *  Created on: 11 de fev de 2016
 *      Author: thiago.palmieri
 */

/* based on a example-code from Keil for CS G++ */

/* for caddr_t (typedef char * caddr_t;) */
#include <sys/types.h>

extern int __HEAP_START;

caddr_t _sbrk (int incr)
{
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;

	if (heap == NULL)
	{
		heap = (unsigned char *)&__HEAP_START;
	}
	prev_heap = heap;
	/* check removed to show basic approach */

	heap += incr;

	return (caddr_t)prev_heap;
}

