#ifndef PARSER_H
#define PARSER_H

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
  bool is_subcmd; // type is program xor literal, no need for enum on a single switch
  union {
    struct _program_t *program;
    char *literal;
  } val;
} leaf;

enum redir_ops { 
  RDR_SEND, // '>'
  RDR_APPEND, // '>>'
  RDR_TAKE, // '<'
};
typedef struct {
  enum redir_ops op;
  char *filename;
} io_redir_t;

typedef struct {
  bool is_redir; // type is word xor redir, no need for enum on a single switch
  union {
    char *word;
    io_redir_t *io_redir;
  } literal;
  struct _cmd_suffix_t *suffix;
} cmd_suffix_t;

typedef struct {
  char *cmd_name;
  cmd_suffix_t *cmd_suffix;
} simple_cmd_t;

typedef struct _pipe_seq_t {
  simple_cmd_t *simple_cmd;
  struct _pipe_seq_t *pipe_seq;
} pipe_seq_t;

typedef struct _program_t {
  pipe_seq_t *pipe_seq;
} program_t;

#endif
