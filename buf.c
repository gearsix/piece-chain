
#include "buf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const size_t blksiz = BUFSIZ/sizeof(Piece);
static Piece **nextp = NULL;

static size_t pfree(Piece *p)
{
	static Piece **freeblk = NULL;
	static size_t nblk = 0, nfree = 0;

	if (!p) {
		if (!nextp && nfree > 0)
			nextp = &freeblk[--nfree];
		return nfree;
	}

	if (nfree + 1 > blksiz * nblk)
		freeblk = realloc(freeblk, (++nblk * blksiz) * sizeof(Piece *));
	if (!freeblk) { perror("failed to allocate freeblk"); exit(1); }

	freeblk[nfree++] = p;
	nextp = &freeblk[nfree - 1];
	return nfree;
}

static Piece *palloc()
{
	Piece *p;
	static Piece *pieceblk = NULL;
	static size_t nblk = 0, npieces = 0;

	if (pfree(NULL) > 0) {
		p = *nextp;
		nextp = NULL;
	} else {
		if (npieces + 1 > blksiz * nblk)
			pieceblk = realloc(pieceblk, (++nblk * blksiz) * sizeof(Piece));
		if (!pieceblk) { perror("failed to allocate pieceblk"); exit(1); }
		p = &pieceblk[npieces++];
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

struct Buf *bufinit(const char *fpath)
{
	Buf *b = calloc(1, sizeof(Buf));

	b->append = tmpfile();
	b->tail = b->pos = b->head = palloc();

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

	pre = bufidx(b, pos);
	post = bufidx(b, (end > b->size) ? b->size : end)->next;
	if (!post) post = b->head;

	pre->next = post;
	post->prev = pre;

	b->idx = pos;
	return (b->size -= num);
}
