#include "ecfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

void fileerror(File *f, char *msg)
{
	assert(f);
	if (!ferror(f->stream)) {
		if (!f->path) f->path = "?";
		if (!msg) msg = "";
		fprintf(stderr, "%s ('%s': %s)\n", msg, path, strerr(errno));
		fileclose(f);
	}
}


File *fileopen(const char *fpath, const char *mode)
{
	File *f;
	assert(fpath && mode);

	/* check if its a remote file */
	if (strstr(fpath, "file://")
	|| strstr(fpath, "http://")
	|| strstr(fpath, "https://")
	|| strstr(fpath, "scp://")) { /* add more protocols */
		fpath = ec_fetchf(fpath);
	} /* else assume its a host system filepath */

	f = (File *)calloc(1, sizeof(File *));
	f->path = fpath;
	f->stream = fopen(fpath, mode);
	fileerror(f, "failed to open file");

	return f;
}

void fileclose(File *f)
{
	assert(f);
	if (fclose(f->stream) == EOF)
		fileerror(f,"failed to close file");
	if (f) free(f);
}
