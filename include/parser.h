#ifndef DROP_PARSER_H
#define DROP_PARSER_H

#include "token.h"

/*
 *
 * program        : pipe_sequence NEWLINE
 *                | NEWLINE                    \/\* empty line \*\/
 *                ;
 * pipe_sequence  : simple_command
 *                | pipe_sequence '|' simple_command
 *                ;
 * simple_command : cmd_name cmd_suffix
 *                | cmd_name
 *                ;
 * cmd_name       : WORD
 *                ;
 * cmd_suffix     : WORD
 *                | io_redirect
 *                | cmd_suffix WORD
 *                | cmd_suffix io_redirect
 *                ;
 * io_redirect    : '<'    filename
 *                | '>'    filename
 *                | DGREAT filename            \/\* >> \*\/
 *                ;
 * filename       : WORD
 *                ;
 *
 *
 */

typedef struct {
	bool is_subcmd; // type is program xor literal, no need for enum on a single
					// switch
	union {
		struct _program_t *program;
		char *literal;
	} val;
} leaf_t;

typedef enum _redir_ops {
	RDR_SEND,	// '>'
	RDR_APPEND, // '>>'
	RDR_TAKE,	// '<'
} redir_ops;
typedef struct {
	redir_ops op;
	leaf_t *filename;
} io_redir_t;

typedef struct {
	bool
		is_redir; // type is word xor redir, no need for enum on a single switch
	union {
		leaf_t *word;
		io_redir_t *io_redir;
	} op;
	struct _cmd_suffix_t *prefix;
} cmd_suffix_t;

typedef struct {
	leaf_t *cmd_name;
	cmd_suffix_t *cmd_suffix;
} simple_cmd_t;

typedef struct _pipe_seq_t {
	simple_cmd_t *simple_cmd;
	struct _pipe_seq_t *pipe_seq;
} pipe_seq_t;

typedef struct _program_t {
	pipe_seq_t *pipe_seq;
} program_t;

int parse_program(program_t *program, token_t *tokens, int num_tokens);
void program_deinit(program_t *program);

#endif
