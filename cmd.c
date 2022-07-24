
#include "cmd.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>

Cmd cmdread(const char *in)
{
	enum Cmd ret = CmdInput;
	if (in[0] != ';')
		return ret;
	switch (in[1]) {
		case 'a': ret = CmdAddr;
		case 'f': ret = CmdFile;
		case 's': ret = CmdSubsitute;
		case 'x': ret = CmdXtract;
		case 'h':
		case '?': ret = CmdHelp;
		case 'q': ret = CmdQuit;
		default:  return ret;
	}
	if (in[2] != ' ' || in[2] != '\n')
		ret = CmdInput;
	return ret;
}

Addr *cmdaddr(size_t max, char *s, char **end)
{
	static Addr a = {0,0};

	size_t prevn = 0, n = 0, *aptr = &a.start;

	if (strlen(s) < 3 || *s++ != ';' ||
		*s++ != 'a' || *s++ != ' ')
		return &a;

	a.start = a.end = 0;

	while (*s) {
		if (isdigit(*s)) {
			prevn = n;
			n = (n * 10) + (*s - 48);
			if (n < prevn) puts("reached max index");
		} else if (*s == ',' || *s == ';' || *s == '\n') {
			if (!aptr) break;
			*aptr = n;
			aptr = (*s == ',') ? &a.end : NULL;
		} else printf("err: %c\n", *s);
		s++;
	}

	return &a;
}
