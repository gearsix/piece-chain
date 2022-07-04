#include "buf.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define INFILE    "in.txt"
#define INBUF     "hello world"
#define OUTFILE "out.txt"
#define OUTBUF "hey buddy!" /* tests will aim to change INBUF to this */

Buf *b;

void setup()
{	
	FILE *in = fopen(INFILE, "w+b");
	if (ferror(in)) perror("fopen in.txt");
	fputs(INBUF, in);
	fclose(in);

	return;
}

void print(Buf *b)
{
	Piece *p = b->tail;

	printf("Buf: %p\n", (void *)b);
	printf("size: %lu, index: %lu\n", b->size, b->idx);
	do {
		printf("%p", (void *)p);
		if (p == b->tail) printf("<-tail");
		if (p == b->pos) printf("<-pos");
		if (p == b->head) printf("<-head");
		puts("");
	} while ((p = p->next));
	printf("\n");
}

void test_bufinit()
{
	Piece *p;
	b = bufinit(INFILE);

	assert(b->read);
	assert(b->append);

	assert(b->pos == b->tail);
	assert(b->pos == b->head);

	p = b->pos;
	assert(p);
	assert(p->f == b->read);
	assert(p->off == 0);
	assert(p->len == strlen(INBUF));
	assert(p->prev == NULL);
	assert(p->next == NULL);
}

void test_bufidx()
{
	Piece *p; size_t idx = 5;
	p = bufidx(b, idx);

	assert(p == b->pos);
	assert(p == b->head);
	assert(p->prev == b->tail);
	assert(p->f == b->read);
	assert(p->off == idx);
	assert(p->len == strlen(INBUF) - idx);
}

void test_bufins()
{
	const char *buf = "y";
	const size_t idx = 2, len = strlen(buf);
	bufins(b, idx, buf);

	assert(b->size == strlen(INBUF) + len);
	assert(b->idx == idx + len);
	assert(b->pos == b->tail->next->next);
	assert(b->pos->f == b->read);
	assert(b->pos->off == 2);
	assert(b->pos->len == 3);
	assert(b->pos->undo == NULL);
	assert(b->pos->redo == NULL);
	assert(b->pos->prev == b->tail->next);
	assert(b->pos->next == b->head);
}

int main()
{
	setup();
	test_bufinit();
	test_bufidx();
	test_bufins();
	puts("success - no assertions failed");
	return 0;
}
