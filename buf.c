
#include "buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Piece *palloc()
{
	static Piece *pieceblk = NULL;
	static size_t nblk = 0, npieces = 0;
	static const size_t blksiz = BUFSIZ/sizeof(Piece);

	if (npieces + 1 > blksiz * nblk)
		pieceblk = realloc(pieceblk, (++nblk * blksiz) * sizeof(Piece));
	if (!pieceblk) { perror("failed to allocate pieceblk memory"); exit(1); }
	return &pieceblk[npieces++];
}

Piece *psplit(Piece *p, long int offset)
{
	Piece *q = palloc();
	if (offset > 0) {
		memcpy(q, p, sizeof(Piece));

		q->off += offset;
		q->len -= p->len = offset;
		q->next = p->next;
		q->prev = p;
		p->next = q->next->prev =q;

		p = q;
	}
	return p;
}

struct Buf *bufinit(const char *fpath)
{
	Buf *b = calloc(1, sizeof(Buf));
	
	b->append = tmpfile();

	b->pos = palloc();
	b->pos->prev = b->tail = palloc();
	b->pos->next = b->head = palloc();
	b->head->prev = b->tail->next = b->pos;

	if (fpath) {
		b->read = fopen(fpath, "rb");
		if (ferror(b->read)) perror(fpath);
		
		b->pos->f = b->read;
		fseek(b->read, 0, SEEK_END);
		b->size = b->pos->len = ftell(b->read);
	}

	return b;
}

Piece *bufidx(Buf *b, size_t pos)
{
	size_t idx = b->idx, offset;
	Piece *p = b->pos;

	if (pos >= idx) {
		while (pos >= idx + p->len && p->next) {
			idx += p->len;
			p = p->next;
		}
	} else {
		do {
			p = p->prev;
			idx -= p->len;
		} while(pos > idx && p->prev);
	}

	if (idx == pos) {
		b->idx = idx;
		b->pos = p;
	} else {
		offset = (idx >= pos) ? idx - pos : pos - idx;
		b->pos = psplit(p, offset);
		b->idx = pos;
	}

	return b->pos;
}

void bufins(Buf *b, size_t pos, const char *buf)
{
}
