#include "test.h"
#include "cmd.h"

#include <stdlib.h>
#include <string.h>

static void setup();
static void setdown();
static void test_cmdread();
static void test_cmdaddr();

void test_cmd()
{
	setup();
	test_cmdread();
	test_cmdaddr();
	setdown();
}

static void setup()
{
}

static void setdown()
{
}

static void test_cmdread()
{
	/* might be unccessary */
}

static void test_cmdaddr()
{
	char *input = malloc(BUFSIZ);
	strcpy(input, ";a 123\n\0");
	cmdaddr(333, input, &input);
	strcpy(input, ";a 4294967295\n\0");
	cmdaddr(333, input, &input);
}
