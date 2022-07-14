/* A piece chain implementation.
	gearsix, 2022 */
#include <stdio.h>

/* Points to a file (`f`), which contains a string which starts
	within `f` at `off` and has a length of `len`.
	`prev` and `next` point to the previous and next `Piece`
	elements in the intended doubly-linked list. */
typedef struct Piece {
	FILE *f;
	size_t off, len;
	struct Piece *prev, *next;
} Piece;

/* Holds a doubly-linked `Piece` list, starting at `tail` and ending
	at `head`. `pos` points to the last addressed `Piece`.
	`size` is the sum length of all `Piece` `len` values in the
	chain. `idx` is the last addressed index in the chain.
	`read` and `append` point to the original file (`read`) and
	any data to be added `append`. */
typedef struct Buffer {
	FILE *read, *append;
	size_t size, idx;
	struct Piece *tail, *pos, *head;
} Buffer;

/* Allocates & initialises a `Buffer`. If `append` is NULL, nothing is
	allocated or initialised and NULL is returned. */
Buffer *
bufinit(FILE *read, FILE *append);

/* Frees `b` and all `Piece` items associated with it. */
void
buffree(Buffer *b);

/* Set `b->idx` to `pos` and `b->pos` to the `Piece` in the chain,
	where the start of `b->pos->next` is `pos`. */
Piece *
bufidx(Buffer *b, size_t pos);

/* Adds a new `Piece` to the chain, at `pos` (found using `bufidx`).
	`s` will be appended to `b->append` and the new `Piece` will
	reflect the appended data. */
size_t
bufins(Buffer *b, size_t pos, const char *s);

/* Removed all pieces from index `pos` to `pos+num`.
	`pos` and `pos+num` are found using `bufidx`. */
size_t
bufdel(Buffer *b, size_t pos, int num);

/* writes all data in `b` to `f`. */
int
bufout(Buffer *b, FILE *f);
