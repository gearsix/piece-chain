/*
*/

struct Piece {
	const size_t pos;
	const char * buf;
	struct Piece *prev, next;
};

typedef struct {
	struct File * read;
	size_t size;
	struct Piece *tail, *head;
} Buf;

/* bufins will add `s` to `buf` starting
	at position `pos`.
	The number of written items will be returned. */
size_t bufins(size_t pos, const char *s);

/* ecbuf_rm will remove `n` items from `buf`,
	starting at position `pos`.
	The number of removed items will be returned. */
size_t bufdel(size_t pos, size_t n);
