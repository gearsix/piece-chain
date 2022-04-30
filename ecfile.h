#include <stdio.h>

typedef struct File {
	const char *path;
	FILE *stream;
} File;

// fileopen is a wrapper for `fopen`, providing
// functionality and error-checked required by ec.
// `path` should the path of the file to open, `mode`
// should be the mode to open the file in, this is the
// same `mode` parameter passed to `fopen`.
File *fileopen(const char *path, const char *mode);

// fileclose will close `f` and free any allocated
// data blocks.
void fileclose(File *f);
