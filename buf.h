/* A piece chain implementation.
	gearsix, 2022 */
#ifndef ECBUF
#define ECBUF

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>


/* Points to a file (`f`), which contains a string which starts
	within `f` at `off` and has a length of `len`.
	`prev` and `next` point to the previous and next `Piece`
	elements in the intended doubly-linked list. */
struct Piece {
	FILE *f;
	size_t off, len;
	struct Piece *prev, *next;
};

/* Holds a linked `Piece` list, starting at `tail` and ending
	at `head`. `pos` points to the last addressed `Piece`.
	`size` is the sum length of all `Piece` `len` values in the
	list. `idx` is the last addressed index in the chain.
	`read` and `append` point to the original file (`read`) and
	any data to be added `append`. */
struct Buf {
	FILE *read, *append;
	size_t size, idx;
	struct Piece *tail, *pos, *head;
};


typedef struct Piece Piece;
typedef struct Buf Buf;


/* Allocates & initialises a `Buf`. If `append` is NULL,
	nothing is allocated or initialised and NULL
	is returned. */
Buf *bufinit(FILE *read, FILE *append);

/* Free `b` and all `Piece` items associated with it. */
void buffree(Buf *b);

/* Set `b->idx` to `pos` and `b->pos` to the `Piece` in
	the chain, where the start of `b->pos->next` is `pos`. */
Piece *bufidx(Buf *b, size_t pos);

/* Adds a new `Piece` to the chain, at `pos` (found
	using `bufidx`). `s` will be appended to `b->append`
	and the new `Piece` will reflect the appended data. */
size_t bufins(Buf *b, size_t pos, const char *s);

/* Removed all pieces from index `pos` to `pos+num`.
	`pos` and `pos+num` are found using `bufidx`. */
size_t bufdel(Buf *b, size_t pos, int num);

/* writes all data in `b` to `f`. */
int bufout(Buf *b, FILE *f);


#ifdef __cplusplus
}
#endif

#endif /* ECBUF */
