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
	Piece *p;

	printf("b->tail: %p\tb->pos: %p\tb->head: %p\n\n",
		(void *)b->tail, (void *)b->pos, (void *)b->head);

	for (p = b->tail; p != NULL; p = p->next)
		printf("%p\n", (void *)p);
}

void test_bufinit()
{
	Piece *p;
	b = bufinit(INFILE);

	assert(b->read);
	assert(b->append);

	p = b->tail;
	assert(p);
	assert(p->f == NULL);
	assert(p->off == 0);
	assert(p->len == 0);
	assert(p->next == b->pos);
	assert(p->prev == NULL);

	p = b->pos;
	assert(p);
	assert(p->f == b->read);
	assert(p->off == 0);
	assert(p->len == strlen(INBUF));
	assert(p->prev == b->tail);
	assert(p->next == b->head);

	p = b->head;
	assert(p);
	assert(p);
	assert(p->f == NULL);
	assert(p->off == 0);
	assert(p->len == 0);
	assert(p->next == NULL);
	assert(p->prev == b->pos);
}

void test_bufidx()
{
	Piece *p; size_t idx = 5;
	p = bufidx(b, idx);

	assert(p == b->pos);
	assert(p->f == b->read);
	assert(p->off == idx);
	assert(p->len == strlen(INBUF) - idx);
	assert(p->prev != b->tail);
	assert(p->next == b->head);
}

int main()
{
	setup();
	test_bufinit();
	test_bufidx();
	puts("success - no assertions failed");
	return 0;
}
