#include "eval.h"
#include "drop.h"
#include "parser.h"
#include "scanner.h"
#include <stdlib.h>
#include <string.h>

int eval_line(char *line) {
	scanner_t s;
	int rc = scanner_init(&s, line);
	if (rc == DROP_ERR) {
		perror("scanner_init");
		return rc;
	}

	rc = scan_tokens(&s);
	if (rc == DROP_ERR) {
		scanner_deinit(&s);
		perror("scanner_init");
		return rc;
	}

	program_t p;
	rc = parse_program(&p, s.tokens.tokens, s.tokens.len);
	if (rc == DROP_ERR) {
		scanner_deinit(&s);
		perror("scanner_init");
		return rc;
	}
	print_program(&p);

	scanner_deinit(&s);
	program_deinit(&p);

	if (strcmp(line, "exit\n") == 0) {
		puts("goodbye!");
		free(line);
		return DROP_EXIT;
	}
	return 0;
}
