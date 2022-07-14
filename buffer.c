#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CATCH(condition, msg, action) \
	if (condition) { perror(msg); action; }

static Piece *pieceblk = NULL;
static Piece **freeblk = NULL;

static void pclean()
{
	if (freeblk) free(freeblk);
	if (pieceblk) free(pieceblk);
}

/* adds `p` to `freeblk` and returns the last added `freeblk` item.
	If `p` is NULL, the last added `freeblk` item will be
	returned, or NULL if `freeblk` is empty.
	`freeblk` is dynamically allocated. */
static Piece *pfree(Piece *p)
{
	static size_t nblk = 0, nfree = 0, blksiz = 0;
	static const size_t allocsiz = BUFSIZ/sizeof(Piece *);

	if (!p) return (nfree == 0) ? NULL : freeblk[--nfree];

	if (nfree + 1 > allocsiz * nblk) {
		blksiz = (++nblk * allocsiz) * sizeof(Piece *);
		freeblk = realloc(freeblk, blksiz);
	}
	CATCH(!freeblk, "failed to allocate freeblk",
		exit(EXIT_FAILURE));

	return freeblk[nfree++] = p;
}

/* returns the next available `Piece` item in `pieceblk`.
	If `freeblk` is not empty, it's last item pointed to by it
	will be returned (recycling `pieceblk` items). The returned
	`Piece` will always have it's values set to 0.
	`pieceblk` is dynamically allocated. */
static Piece *palloc()
{
	Piece *p;
	
	static size_t nblk = 0, nalloc = 0, blksiz = 0;
	static const size_t allocsiz = BUFSIZ/sizeof(Piece);

	static int hook = 0;

	if ( !(p = pfree(NULL)) ) {
		if (nalloc + 1 > allocsiz * nblk) {
			blksiz = (++nblk * allocsiz) * sizeof(Piece);
			pieceblk = realloc(pieceblk, blksiz);
		}
		CATCH(!pieceblk, "failed to allocate pieceblk",
			exit(EXIT_FAILURE));
		if (hook == 0) hook = !atexit(pclean);
		
		p = &pieceblk[nalloc++];
	}

	return memset(p, 0, sizeof(Piece));
}

/* splits `p` into two `Piece` items. The values of `p` will be
	modified to fit the first `Piece`; a new `Piece` will be
	allocated for the second.
	The `prev` and `next` values of these & associated `Piece`
	items will be updated to reflect the split.
	`offset` should be the offset within `p->len` to split at.
	If offset is not within the boundry of `p->len`, then `p`
	will be returned. The *first* `Piece` in the split is
	returned. */
static Piece *psplit(Piece *p, long int offset)
{
	Piece *q = p;
	if (offset > 0 && offset <= (int)p->len) {
		q = palloc();
		memcpy(q, p, sizeof(Piece));
		q->off += offset;
		q->len -= p->len = offset;
		q->next = p->next;
		q->prev = p;
		p->next = q;
		if (q->next) q->next->prev = q;
	} else if (offset == 0) {
		q = palloc();
		q->next = p;
		p->prev = q;
	}
	return q;
}

/* pfind will find the `Piece` which contains index `pos`.
	The search starts from `p`, where the index is `idx`.
	`p` will be set to the found `Piece`.
	The new index will be returned. */
static size_t pfind(Piece **ptr, size_t idx, size_t pos)
{
	Piece *p = *ptr;
	if (pos >= idx) {
		while (pos >= idx + p->len && p->next) {
			idx += p->len;
			p = p->next;
		}
	} else {
		do {
			p = p->prev;
			idx -= p->len;
		} while (pos < idx && p->prev);
	}
	*ptr = p;
	return idx;
}

struct Buffer *bufinit(FILE *read, FILE *append)
{
	Buffer *b = calloc(1, sizeof(Buffer));

	if (!append) return NULL;
	
	b->read = read;
	b->append = append;
	
	b->tail = b->pos = b->head = palloc();
	b->pos->f = b->read;
	fseek(b->read, 0, SEEK_END);
	b->size = b->pos->len = ftell(b->read);

	return b;
}

void buffree(Buffer *b)
{
	Piece *p = b->tail->next;
	while (p) {
		pfree(p->prev);
		p = p->next;
	}
	free(b);
}

Piece *bufidx(Buffer *b, size_t pos)
{
	size_t offset, idx = pfind(&b->pos, b->idx, pos);

	if (idx != pos || idx == 0) {
		offset = (idx >= pos) ? idx - pos : pos - idx;
		b->pos = psplit(b->pos, offset);
		if (!b->pos->prev) b->tail = b->pos;
		if (!b->pos->next) b->head = b->pos;
	}
	
	b->idx = pos;
	return b->pos;
}

size_t bufins(Buffer *b, size_t pos, const char *s)
{
	const size_t slen = strlen(s);
	Piece *p;

	b->pos = bufidx(b, pos)->prev;
	if (!b->pos) b->pos = b->tail;
	
	p = palloc();
	p->f = b->append;
	p->off = ftell(b->append);
	p->len = slen;
	p->prev = b->pos;
	p->next = b->pos->next;

	b->pos->next = b->pos->next->prev = p;

	fprintf(b->append, "%s", s);
	CATCH(ferror(b->append), "bufins: write to append",
		{ pfree(p); p = 0; });

	if (p != 0) {
		b->idx += slen;
		b->size += slen;
		b->pos = p->next;
	}

	return b->size;
}

size_t bufdel(Buffer *b, size_t pos, int num)
{
	size_t tmp;
	Piece *pre, *post;

	if (num < 0) {
		if ((int)pos-num < 0)
			num = pos;
		else {
			tmp = abs(num);
			num = pos;
			pos = (tmp > pos) ? 0 : tmp;
		}
	}

	if (pos+num > b->size) num = b->size - pos;
	
	pre = bufidx(b, pos)->prev;
	if (!pre) pre = b->tail;
	post = bufidx(b, pos+num);

	pre->next = post;
	post->prev = pre;

	b->idx = pos;
	return (b->size -= num);
}

int bufout(Buffer *b, FILE *f)
{
	size_t n, fsiz = 0;
	char buf[BUFSIZ];
	Piece *p = b->tail;

	do {
		if (p->len > 0 && p->f) {
			if ((size_t)ftell(p->f) != p->off)
			fseek(p->f, p->off, SEEK_SET);
		
			n = 0;
			do {
				buf[0] = '\0';
				fread(buf, 1, p->len, p->f);
				CATCH(ferror(p->f), "bufout: fread failed", break);
				fsiz += fwrite(buf, 1, p->len, f);
				CATCH(ferror(f), "bufout: fwrite failed", break);
			} while (p->len - (BUFSIZ*n++) > BUFSIZ);
		}
		p = p->next;
	} while(p);

	return fsiz;
}
