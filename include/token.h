#ifndef DROP_TOKEN_H
#define DROP_TOKEN_H

#include <stddef.h>

typedef enum {
	TOKEN_WORD,	   // ls, -la, foo\ bar, $HOME
	TOKEN_STRING,  // "text"
	TOKEN_NEWLINE, // \n: ends a command
	TOKEN_EOF,

	TOKEN_PIPE,	  // |
	TOKEN_LESS,	  // <   (your LEFT_ARROW)
	TOKEN_GREAT,  // >   (your RIGHT_ARROW)
	TOKEN_DGREAT, // >>  append

	// far out
	TOKEN_SEMI,	  // ;
	TOKEN_AMP,	  // &   background
	TOKEN_AND_IF, // &&
	TOKEN_OR_IF,  // ||

	// probably won't get to
	TOKEN_IO_NUMBER, // the 2 in 2>file (digits directly before < or >)
	TOKEN_GREATAND,	 // >&  as in 2>&1
	TOKEN_LESSAND,	 // <&
	TOKEN_DLESS,	 // <<  heredoc
	TOKEN_LPAREN,	 // (   subshell
	TOKEN_RPAREN,	 // )
} token_type_t;

typedef struct {
	token_type_t type;
	const char *start;
	size_t len;

	// INFO: may add line if we eventually support scripts and other things
	// int line;
} token_t;

typedef struct {
	token_t *tokens;
	size_t cap;
	size_t len;
} token_arr;

int token_arr_init(token_arr *arr);
int token_arr_push(token_arr *arr, token_t token);
void token_arr_deinit(token_arr *arr);

#endif // !DROP_TOKEN_H
