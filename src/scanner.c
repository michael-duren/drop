#include "scanner.h"
#include "drop.h"
#include "token.h"
#include <asm-generic/errno-base.h>
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int scanner_init(scanner_t *s, char *src) {
	if (!s || !src) {
		errno = EINVAL;
		return DROP_ERR;
	}
	int rc = token_arr_init(&s->tokens);
	if (rc != DROP_OK) {
		return rc;
	}

	s->source = src;
	s->source_len = strlen(src);
	s->current = src;
	s->start = src;

	return DROP_OK;
}

void scanner_deinit(scanner_t *s) {
	if (!s) {
		return;
	}
	token_arr_deinit(&s->tokens);
}

char advance(scanner_t *s) { return *s->current++; }

bool at_end(scanner_t *s) { return s->current >= s->source_len + s->source; }
char peek(scanner_t *s) {
	if (at_end(s))
		return '\0';
	return *s->current;
}

bool validword_c(char c) { return isalnum(c) || c == '-' || c == '_'; }

bool is_operator(char c) {
	switch (c) {
	case '|':
	case '<':
	case '>':
	case '(':
	case ')':
		return true;
	}

	return false;
}

int add_token(scanner_t *s, token_type_t type) {
	switch (type) {
	case TOKEN_WORD:
		while (true) {
			char curr = peek(s);
			if (isspace(curr) || at_end(s) || is_operator(curr)) {
				break;
			}
			if (!validword_c(curr)) {
				errno = EINVAL;
				return DROP_ERR;
			}
			advance(s);
		}
		break;
	case TOKEN_STRING:
		while (true) {
			char curr = peek(s);
			if (curr == '"') {
				advance(s);
				break;
			}
			// we didn't find the other "
			if (at_end(s)) {
				errno = EINVAL;
				return DROP_ERR;
				break;
			}
			advance(s);
		}
		break;
	default:
		break;
	}
	return token_arr_push(&s->tokens,
						  (token_t){.len = (size_t)(s->current - s->start),
									.start = s->start,
									.type = type});
}

int scan_token(scanner_t *s) {
	char c = advance(s);
	int rc = DROP_OK;
	switch (c) {
	case ' ':
	case '\t':
		break;
	case '\n':
		rc = add_token(s, TOKEN_NEWLINE);
		break;
	case '|':
		rc = add_token(s, TOKEN_PIPE);
		break;
	case '<':
		// TODO: needs more checks
		rc = add_token(s, TOKEN_LESS);
		break;
	case '>':
		// TODO: needs more checks
		rc = add_token(s, TOKEN_GREAT);
		break;
	case '(':
		rc = add_token(s, TOKEN_LPAREN);
		break;
	case ')':
		rc = add_token(s, TOKEN_RPAREN);
		break;
	case '"':
		rc = add_token(s, TOKEN_STRING);
		break;
	default:
		rc = add_token(s, TOKEN_WORD);
		break;
	}

	return rc;
}

int scan_tokens(scanner_t *s) {
	if (s == NULL) {
		errno = EINVAL;
		return DROP_ERR;
	}

	while (!at_end(s)) {
		s->start = s->current;
		int rc;
		if ((rc = scan_token(s)) != DROP_OK) {
			return rc;
		}
	}
	s->start = s->current;
	add_token(s, TOKEN_EOF);

	return DROP_OK;
}
