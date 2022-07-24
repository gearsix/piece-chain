/* The ec command language.
	gearsix, 2022 */
#ifndef ECCMD
#define ECCMD

#ifdef __cplusplus
#extern "C" {
#endif

#include "buf.h"


#define ADDR_IDX '#'
#define ADDR_ROW '.'
#define ADDR_COL ':'


typedef enum Cmd {
	CmdInput,
	CmdAddr,
	CmdFile,
	CmdSubsitute,
	CmdXtract,
	CmdHelp,
	CmdQuit
} Cmd;

typedef struct Addr {
	size_t start, end;
} Addr;


Cmd cmdread(const char *in);

Addr *cmdaddr(size_t bufsiz, char *s, char **end);


#ifdef __cplusplus
}
#endif

#endif /* ECCMD */
