#ifndef DROP_SCANNER_H
#define DROP_SCANNER_H

#include "token.h"
#include <stddef.h>
#include <stdio.h>

typedef struct {
	char *source;
	size_t source_len;
	token_arr tokens;
	// current place when scanning
	char *current;
	// starting place of the token scan
	char *start;
	// INFO: may add line if we eventually support scripts and other things
	// int line;
} scanner_t;

int scan_tokens(scanner_t *s);

#endif // !DROP_SCANNER_H
