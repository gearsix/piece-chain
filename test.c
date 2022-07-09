#include "buf.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define INFILE    "in.txt"
#define INBUF     "hello world"
#define OUTFILE "out.txt"
#define OUTBUF "hey buddy!" /* tests will aim to change INBUF to this */

Buf *b;
FILE *in, *tmp, *out;

void setup()
{	
	in = fopen(INFILE, "w+b");
	if (ferror(in)) perror("fopen in.txt");
	fputs(INBUF, in);

	tmp = tmpfile();

	out = fopen(OUTFILE, "w+b");
	if (ferror(in)) perror("fopen out.txt");

	return;
}

void setdown()
{
	buffree(b);
	fclose(out);
	fclose(in);
	fclose(tmp);
}

void print()
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
		printf("\tf: %s, off: %lu, len %lu\n",
			(p->f == b->read) ? "read" : "append", p->off, p->len);
	} while ((p = p->next));
	printf("\n");
	fflush(stdout);
}

void test_bufinit()
{
	Piece *p;

	b = bufinit(in, tmp);

	assert(b->read);
	assert(b->append);
	assert(b->pos == b->tail);
	assert(b->pos == b->head);

	assert((p = b->pos));
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

void test_bufdel()
{
	const size_t idx = 3, len = 9;

	bufdel(b, idx, len);

	assert(b->size == strlen(INBUF)+1-len); /* +1 for bufins */
	assert(b->idx == idx);
	assert(b->pos == b->head);
	assert(b->tail->next->next == b->pos);

	assert(b->pos->f == b->read);
	assert(b->pos->off == 11);
	assert(b->pos->len == 0);
	assert(b->pos->undo == NULL);
	assert(b->pos->redo == NULL);
	assert(b->pos->prev == b->tail->next);
	assert(b->pos->next == NULL);
}

void test_bufins1()
{
	const char *buf = " buddy!";
	const size_t idx = 3, len = strlen(buf), bsiz = b->size;

	bufins(b, idx, buf);

	assert(b->size == bsiz + len);
	assert(b->idx == idx + len);
	assert(b->pos == b->head);
	assert(b->pos == b->tail->next->next->next);

	assert(b->pos->prev->f == b->append);
	assert(b->pos->prev->off == 1);
	assert(b->pos->prev->len == len);
	assert(b->pos->undo == NULL);
	assert(b->pos->redo == NULL);
	assert(b->pos->prev == b->tail->next->next);
	assert(b->pos->next == NULL);
}

void test_bufout()
{
	char buf[BUFSIZ] = {0};

	bufout(b, out);

	rewind(out);
	assert(fread(buf, 1, BUFSIZ, out) > 0);
	assert(strcmp(buf, OUTBUF) == 0);
}

int main()
{
	setup();
	test_bufinit();
	test_bufidx();
	test_bufins();
	test_bufdel();
	test_bufins1();
	test_bufout();
	setdown();
	puts("success - no assertions failed");
	return 0;
}
