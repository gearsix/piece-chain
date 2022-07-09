
#include "buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Piece *pieceblk = NULL; /* block of all pieces */
static Piece **freeblk = NULL; /* ptr block to all free'd `pieceblk` items */

static void pclean()
{
	free(freeblk);
	free(pieceblk);
}

static Piece *pfree(Piece *p)
{
	static size_t nblk = 0, nfree = 0;
	static const size_t blksiz = BUFSIZ/sizeof(Piece *);

	if (!p) return (nfree == 0) ? NULL : freeblk[--nfree];

	if (nfree + 1 > blksiz * nblk)
		freeblk = realloc(freeblk, (++nblk * blksiz) * sizeof(Piece *));
	if (!freeblk) { perror("failed to allocate freeblk"); exit(1); }

	return freeblk[nfree++] = p;
}

static Piece *palloc()
{
	Piece *p;
	
	static size_t nblk = 0, nalloc = 0;
	static const size_t blksiz = BUFSIZ/sizeof(Piece);

	static int hook = 0;
	if (hook == 0) { atexit(pclean); hook = 1; }

	if ( !(p = pfree(NULL)) ) {
		if (nalloc + 1 > blksiz * nblk)
			pieceblk = realloc(pieceblk, (++nblk * blksiz) * sizeof(Piece));
		if (!pieceblk) { perror("failed to allocate pieceblk"); exit(1); }
		p = &pieceblk[nalloc++];
	}

	return memset(p, 0, sizeof(Piece));
}

static Piece *psplit(Piece *p, long int offset)
{
	Piece *q = palloc();
	if (offset > 0) {
		memcpy(q, p, sizeof(Piece));

		q->off += offset;
		q->len -= p->len = offset;
		q->next = p->next;
		q->prev = p;
		p->next = q;
		if (q->next) q->next->prev = q;

		p = q;
	}
	return p;
}

struct Buf *bufinit(FILE *read, FILE *append)
{
	Buf *b = calloc(1, sizeof(Buf));

	if (!read || !append) return NULL;
	
	b->read = read;
	b->append = append;
	
	b->tail = b->pos = b->head = palloc();
	b->pos->f = b->read;
	fseek(b->read, 0, SEEK_END);
	b->size = b->pos->len = ftell(b->read);

	return b;
}

void buffree(Buf *b)
{
	Piece *p = b->tail->next;
	while (p) {
		pfree(p->prev);
		p = p->next;
	}
	free(b);
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
		if (!b->pos->next) b->head = b->pos;
		b->idx = pos;
	}

	return b->pos;
}

size_t bufins(Buf *b, size_t pos, const char *s)
{
	const size_t slen = strlen(s);
	Piece *p = palloc();

	b->pos = bufidx(b, pos)->prev;

	p->f = b->append;
	p->off = ftell(b->append);
	p->len = slen;
	p->undo = NULL;
	p->redo = NULL;
	p->prev = b->pos;
	p->next = b->pos->next;

	b->pos->next = b->pos->next->prev = p;

	fprintf(b->append, "%s", s);
	if (ferror(b->append)) {
		perror("failed to write to append file");
		pfree(p);
		p = 0;
	} else {
		b->idx += slen;
		b->pos = p->next;
	}

	return (p == 0) ? b->size : (b->size += slen);
}

size_t bufdel(Buf *b, size_t pos, size_t num)
{
	Piece *pre, *post;
	size_t end = pos+num;

	pre = bufidx(b, (pos == 0) ? 0 : pos-1);
	post = bufidx(b, (end > b->size) ? b->size : end)->next;
	if (!post) post = b->head;

	pre->next = post;
	post->prev = pre;

	b->idx = pos;
	return (b->size -= num);
}

int bufout(Buf *b, FILE *f)
{
	size_t n, fsiz = 0;
	char buf[BUFSIZ];
	Piece *p = b->tail;

	do {
		if ((size_t)ftell(p->f) != p->off)
			fseek(p->f, p->off, SEEK_SET);
		
		n = 0;
		do {
			buf[0] = '\0';
			fread(buf, 1, p->len, p->f);
			fsiz += fwrite(buf, 1, p->len, f);
		} while (p->len - (BUFSIZ*n++) > BUFSIZ);

		p = p->next;
	} while(p);

	return fsiz;
}
