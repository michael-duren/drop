#include "parser.h"
#include "drop.h"
#include <errno.h>
#include <math.h>
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
  if (leaf->is_subcmd) {
    program_deinit(leaf->val.program);
  } else {
    free(leaf->val.literal);
  }
}


