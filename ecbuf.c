#include "ecbuf.h"
#include "ecfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

Buf *bufinit(File *f)
{
	Buf *b = calloc(1, sizeof(Buf));
	b->read = f;
	fseek(f->stream, 0, SEEK_END);
	b->size = ftell(f->stream);
	return b;
}

size_t bufins(Buf *b, size_t pos, const char *s)
{
	struct Piece *p = malloc(sizeof(struct Piece *));
	memcpy(p, &(struct Piece){pos, s, b->head, b->head->next},
		sizeof(struct Piece));
	if (!b->tail) b->tail = p;
	b->head = p;
	return ++b->size;
}

size_t bufdel(size_t pos, size_t n)
{	
}
