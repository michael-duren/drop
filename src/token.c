#include "token.h"
#include "drop.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdlib.h>

#define TOKEN_ARR_INIT_SIZE 8

int token_arr_init(token_arr *arr) {
	if (arr == NULL) {
		errno = EINVAL;
		return DROP_ERR;
	}

	size_t cap = TOKEN_ARR_INIT_SIZE;
	token_t *tokens = malloc(sizeof(token_t) * cap);
	if (tokens == NULL) {
		return DROP_ERR;
	}
	arr->tokens = tokens;
	arr->cap = cap;
	arr->len = 0;

	return DROP_OK;
}

int token_arr_push(token_arr *arr, token_t token) {
	if (arr->cap == arr->len) {
		size_t ncap = arr->cap * 2;
		token_t *tokens = realloc(arr->tokens, ncap * sizeof(token_t));
		if (tokens == NULL) {
			return DROP_ERR;
		}
		arr->cap = ncap;
		arr->tokens = tokens;
	}
	arr->tokens[arr->len] = token;
	arr->len++;
	return DROP_OK;
}

void token_arr_deinit(token_arr *arr) {
	if (!arr) {
		return;
	}
	free(arr->tokens);
}

bool is_rdr_type(token_type_t type) {
	return type == TOKEN_LESS || type == TOKEN_GREAT || type == TOKEN_DGREAT;
}
