#include "parser.h"
#include "drop.h"
#include "token.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int parse_leaf(leaf_t *leaf, token_t *tokens, int num_tokens) {
  if (!leaf || !tokens || !num_tokens) {
    errno = EINVAL;
    return DROP_ERR;
  }

  if (num_tokens == 1 && tokens->type == TOKEN_WORD) {

    leaf->val.literal = malloc(sizeof(char) * (tokens->len + 1));
    if (!leaf->val.literal) {
      errno = ENOMEM;
      return DROP_ERR;
    }

    strncpy(leaf->val.literal, tokens->start, tokens->len);
    return DROP_OK;
  }

  /* This knowingly skips the '$' operator.
   * Don't have a token for it and it isn't unique, 
   * as $HOME is considered TOKEN_WORD.
   * Out of scope for now.
   */
  if (num_tokens > 1 
      && tokens->type == TOKEN_LPAREN
      && tokens[num_tokens-1].type == TOKEN_RPAREN) {

    leaf->is_subcmd = true;

    leaf->val.program = malloc(sizeof(program_t));
    if (!leaf->val.program) {
      errno = ENOMEM;
      return DROP_ERR; 
    }

    return parse_program(leaf->val.program, &tokens[1], num_tokens - 2);
  }
      
  errno = EINVAL;
  return DROP_ERR;
}

void leaf_deinit(leaf_t *leaf) {
  if (!leaf) return;

  if (leaf->is_subcmd) {
    program_deinit(leaf->val.program);
  } else {
    free(leaf->val.literal);
  }
}

int parse_io_redir(io_redir_t *io_redir, token_t *tokens, int num_tokens) {
  if (!io_redir || !tokens || num_tokens != 2) {
    errno = EINVAL;
    return DROP_ERR;
  }

  switch (tokens->type) {
  case TOKEN_LESS:
    io_redir->op = RDR_TAKE;
    break;
  case TOKEN_GREAT:
    io_redir->op = RDR_SEND;
    break;
  case TOKEN_DGREAT:
    io_redir->op = RDR_APPEND;
    break;
  default:
    errno = EINVAL;
    return DROP_ERR;
  }

  int fname_len = tokens[1].len;
  io_redir->filename = malloc(sizeof(char) * (fname_len + 1));
  if (!io_redir->filename) {
    errno = ENOMEM;
    return DROP_ERR;
  }

  strncpy(io_redir->filename, tokens[1].start, fname_len);
  return DROP_OK;
}

void io_redir_deinit(io_redir_t *io_redir) {
  if (io_redir && io_redir->filename) {
    free(io_redir->filename);
  }
}

/*
int parse_cmd_suffix(cmd_suffix_t *cmd_suffix, token_t *tokens, int num_tokens) {

}

void cmd_suffix_deinit(cmd_suffix_t *cmd_suffix) {

}

int parse_simple_cmd(simple_cmd_t *simple_cmd, token_t *tokens, int num_tokens) {

}

void simple_cmd_deinit(simple_cmd_t *simple_cmd) {

}

int parse_pipe_seq(pipe_seq_t *pipe_seq, token_t *tokens, int num_tokens) {

}

void pipe_seq_deinit(pipe_seq_t *pipe_seq) {

}

int parse_program(program_t *program, token_t *tokens, int num_tokens) {

}

void program_deinit(program_t *program) {

}

*/
