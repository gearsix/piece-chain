/* TODO */
#include <stdio.h>

typedef struct Piece {
	FILE *f;
	size_t off, len;
	
	struct Piece *undo, *redo;
	struct Piece *prev, *next;
} Piece;

typedef struct Buf {
	FILE *read, *append;
	size_t size, idx;
	struct Piece *tail, *pos, *head;
} Buf;

Buf *bufinit(const char *fpath);

Piece *bufidx(Buf *b, size_t pos);

size_t bufins(Buf *b, size_t pos, const char *s);
