#include "test.h"
#include "buffer.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

static Buffer *B;

static FILE *in_f;
static const char *in_p = "in.txt", *in_buf = "hello world";
static FILE *out_f;
static const char *out_p = "out.txt", *out_buf = "Hey buddy!";
static FILE *tmp_f;
static const char *tmp_p = "tmp.txt";


static void setup();
static void setdown();
static void test_bufinit();
static void test_bufidx();
static void test_bufins();
static void test_bufdel();
static void test_bufins1();
static void test_bufdel1();
static void test_bufins2();
static void test_bufout();


void test_buffer()
{
	printf("test_buffer: ");
	setup();	
	signal(SIGABRT, setdown);
	test_bufinit();
	test_bufidx();
	test_bufins();
	test_bufdel();
	test_bufins1();
	test_bufdel1();
	test_bufins2();
	test_bufout();
	setdown(0);
	puts("success");
}

static void debugprint()
{
	Piece *p = B->tail;

	printf("\nBuf: %p\n", (void *)B);
	printf("size: %d, index: %d\n", (int)B->size, (int)B->idx);
	do {
		printf("%p", (void *)p);
		if (p == B->tail) printf("<-tail");
		if (p == B->head) printf("<-head");
		if (p == B->pos) printf("<-pos");
		printf("\n\toff: %d, len %d, f: %s\n", (int)p->off, (int)p->len,
			(!p->f) ? "0" : (p->f == B->read) ? "read" : "append");
	} while ((p = p->next));
	printf("\n");
}

static void setup()
{	
	in_f = fopen(in_p, "w+b");
	if (ferror(in_f)) perror("fopen in");
	fputs(in_buf, in_f);

	out_f = fopen(out_p, "w+b");
	if (ferror(out_f)) perror("fopen out");

	tmp_f = fopen(tmp_p, "w+b");
	if (ferror(tmp_f)) perror("fopen tmp");
}

static void setdown(int n)
{
	buffree(B);
	fclose(out_f);
	fclose(in_f);
	fclose(tmp_f);
	if (n != SIGABRT) {
		remove(in_p);
		remove(out_p);
		remove(tmp_p);
	}
}

static void test_bufinit()
{
	Piece *p;

	B = bufinit(in_f, tmp_f);

	assert(B->read);
	assert(B->append);
	assert(B->pos == B->tail);
	assert(B->pos == B->head);

	assert((p = B->pos));
	assert(p->f == B->read);
	assert(p->off == 0);
	assert(p->len == strlen(in_buf));
	assert(p->prev == NULL);
	assert(p->next == NULL);
}

static void test_bufidx()
{
	Piece *p; size_t idx = 5;

	p = bufidx(B, idx);

	assert(p == B->pos);
	assert(p == B->head);
	assert(p->prev == B->tail);
	assert(p->f == B->read);
	assert(p->off == idx);
	assert(p->len == strlen(in_buf) - idx);
}

static void test_bufins()
{
	const char *buf = "y"; size_t idx = 2;
	const size_t len = strlen(buf);

	bufins(B, idx, buf);

	assert(B->size == strlen(in_buf) + len);
	assert(B->idx == idx + len);
	assert(B->pos == B->tail->next->next);

	assert(B->pos->f == B->read);
	assert(B->pos->off == 2);
	assert(B->pos->len == 3);
	assert(B->pos->prev == B->tail->next);
	assert(B->pos->next == B->head);
}

static void test_bufdel()
{
	size_t idx = 3; int len = B->size+1;
	const size_t bsiz = B->size;

	bufdel(B, idx, len);
	
	assert(B->size == bsiz-(bsiz-idx));
	assert(B->idx == idx);
	assert(B->pos == B->head);
	assert(B->tail->next->next == B->pos);

	assert(B->pos->f == B->read);
	assert(B->pos->off == 11);
	assert(B->pos->len == 0);
	assert(B->pos->prev == B->tail->next);
	assert(B->pos->next == NULL);
}

static void test_bufins1()
{
	size_t idx = 3; const char *buf = " buddy!";
	const size_t len = strlen(buf), bsiz = B->size;

	bufins(B, idx, buf);

	assert(B->size == bsiz + len);
	assert(B->idx == idx + len);
	assert(B->pos == B->head);
	assert(B->pos == B->tail->next->next->next);

	assert(B->pos->prev->f == B->append);
	assert(B->pos->prev->off == 1);
	assert(B->pos->prev->len == len);
	assert(B->pos->prev == B->tail->next->next);
	assert(B->pos->next == NULL);
}

static void test_bufdel1()
{
	size_t idx = 0; int len = 1;
	const size_t bsiz = B->size;

	bufdel(B, idx, len);

	assert(B->size == bsiz - len);
	assert(B->idx == idx);
	
	assert(B->pos->prev == B->tail);
	assert(B->pos->prev->f == 0);
	assert(B->pos->prev->off == 0);
	assert(B->pos->prev->len == 0);
	assert(B->pos->prev->prev == 0);
	assert(B->pos->prev->next == B->pos);

	assert(B->pos->f == B->read);
	assert(B->pos->off == 1);
	assert(B->pos->len == (size_t)len);
	assert(B->pos->prev == B->tail);
	assert(B->pos->next->next->next == B->head);
}

static void test_bufins2()
{
	const char *buf = "H"; const size_t idx = 0;
	size_t len = strlen(buf), bsiz = B->size;

	bufins(B, idx, buf);

	assert(B->size == bsiz + len);
	assert(B->idx == idx + len);
	assert(B->pos == B->tail->next->next);

	assert(B->pos->prev == B->tail->next);
	assert(B->pos->prev->f == B->append);
	assert(B->pos->prev->off == 8);
	assert(B->pos->prev->len == 1);
	assert(B->pos->prev->prev == B->tail);
	assert(B->pos->prev->next == B->pos);
}

static void test_bufout()
{
	size_t n = 0;
	char buf[BUFSIZ] = {0};

	bufout(B, out_f);
	rewind(out_f);
	n = fread(buf, 1, BUFSIZ, out_f);
	assert(n > 0);
	assert(strcmp(buf, out_buf) == 0);
}
