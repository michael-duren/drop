#include "parser.h"
#include "drop.h"
#include "token.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int parse_leaf(leaf_t *leaf, token_t *tokens, int num_tokens) {
	if (!leaf || !tokens || num_tokens <= 0) {
		errno = EINVAL;
		return DROP_ERR;
	}

	if (num_tokens == 1 && tokens->type != TOKEN_WORD) {
		errno = EINVAL;
		return DROP_ERR;
	}

	if (num_tokens == 1) {
		leaf->val.literal = calloc(tokens->len + 1, sizeof(char));
		if (!leaf->val.literal) {
			errno = ENOMEM;
			return DROP_ERR;
		}

		memcpy(leaf->val.literal, tokens->start, tokens->len);
		leaf->val.literal[tokens->len] = '\0';
	} else if (num_tokens > 1 && tokens->type == TOKEN_LPAREN &&
			   tokens[num_tokens - 1].type == TOKEN_RPAREN) {
		/* This knowingly skips the '$' operator.
		 * Don't have a token for it and it isn't unique,
		 * as $HOME is considered TOKEN_WORD.
		 * Out of scope for now.
		 */

		leaf->is_subcmd = true;

		leaf->val.program = calloc(1, sizeof(program_t));
		if (!leaf->val.program) {
			errno = ENOMEM;
			return DROP_ERR;
		}

		if (parse_program(leaf->val.program, &tokens[1], num_tokens - 2) ==
			DROP_ERR) {
			free(leaf->val.program);
			return DROP_ERR;
		}
	} else {
		errno = EINVAL;
		return DROP_ERR;
	}

	return DROP_OK;
}

void leaf_deinit(leaf_t *leaf) {
	if (!leaf)
		return;

	if (leaf->is_subcmd) {
		program_deinit(leaf->val.program);
		free(leaf->val.program);
	} else {
		free(leaf->val.literal);
	}
}

int parse_io_redir(io_redir_t *io_redir, token_t *tokens, int num_tokens) {
	if (!io_redir || !tokens || num_tokens < 2 ||
		tokens[1].type != TOKEN_WORD) {
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

	io_redir->filename = calloc(1, sizeof(leaf_t));
	if (!io_redir->filename) {
		errno = ENOMEM;
		return DROP_ERR;
	}
	if (parse_leaf(io_redir->filename, &tokens[1], num_tokens - 1) ==
		DROP_ERR) {
		free(io_redir->filename);
		return DROP_ERR;
	}

	return DROP_OK;
}

void io_redir_deinit(io_redir_t *io_redir) {
	if (io_redir && io_redir->filename) {
		leaf_deinit(io_redir->filename);
		free(io_redir->filename);
	}
}

// Expects tokens ptr to be a left parenthesis
int get_right_paren(token_t *tokens, int tokens_len) {
  if (tokens[0].type != TOKEN_LPAREN) {
    return -1;
  }

	int prn_count = 0;
	for (int i = tokens_len; i < tokens_len; ++i) {
		token_type_t tt = tokens[i].type;

		if (tt == TOKEN_LPAREN) {
			++prn_count;
		} else if (tt == TOKEN_RPAREN) {
			if (prn_count == 0) {
				return i;
			}
			--prn_count;
		}
	}
	return -1;
}

// Expects r_idx to be a right parenthesis
int get_left_paren(token_t *tokens, int r_idx) {
  if (tokens[r_idx].type != TOKEN_RPAREN) {
    return -1;
  }

	int prn_count = 0;
	for (int i = r_idx; i >= 0; --i) {
		token_type_t tt = tokens[i].type;

		if (tt == TOKEN_RPAREN) {
			++prn_count;
		} else if (tt == TOKEN_LPAREN) {
			if (prn_count == 0) {
				return i;
			}
			--prn_count;
		}
	}
	return -1;
}

void cmd_suffix_deinit(cmd_suffix_t *cmd_suffix) {
	if (!cmd_suffix)
		return;

	if (cmd_suffix->is_redir) {
		io_redir_deinit(cmd_suffix->op.io_redir);
		free(cmd_suffix->op.io_redir);
	} else {
		leaf_deinit(cmd_suffix->op.word);
		free(cmd_suffix->op.word);
	}

	cmd_suffix_deinit((cmd_suffix_t *)cmd_suffix->prefix);
	free(cmd_suffix->prefix);
}

int parse_cmd_suffix(cmd_suffix_t *cmd_suffix, token_t *tokens,
					 int num_tokens) {
	if (!cmd_suffix || !tokens || num_tokens <= 0) {
		errno = EINVAL;
		return DROP_ERR;
	}
	// Indicates last node/suffix is a subcommand and not a literal word
	bool last_suffix_subcmd = tokens[num_tokens - 1].type == TOKEN_RPAREN;
	// Index of the starting token of the last node/suffix
	int last_node_pos = -1;
	// Index of redirect token, just beOre last_node_pos if it exists
	int rdr_tkn = -1;
	// Index of the end of the prefix, either just before last_node_pos or
	// rdr_tkn
	int prefix_end_pos = -1;

	if (last_suffix_subcmd) {
		last_node_pos = get_left_paren(tokens, num_tokens - 1);
		if (last_node_pos == -1) {
			errno = EINVAL;
			return DROP_ERR;
		}
	} else {
		last_node_pos = num_tokens - 1;
	}

	if (last_node_pos > 0 && is_rdr_type(tokens[last_node_pos - 1].type)) {
		rdr_tkn = last_node_pos - 1;
		prefix_end_pos = rdr_tkn - 1;
	} else {
		prefix_end_pos = last_node_pos - 1;
	}

	if (rdr_tkn > -1) {
		cmd_suffix->op.io_redir = calloc(1, sizeof(io_redir_t));
		if (!cmd_suffix->op.io_redir) {
			errno = ENOMEM;
			return DROP_ERR;
		}

		cmd_suffix->is_redir = true;
		if (parse_io_redir(cmd_suffix->op.io_redir, &tokens[rdr_tkn],
						   num_tokens - rdr_tkn) == DROP_ERR) {
			cmd_suffix_deinit(cmd_suffix);
			return DROP_ERR;
		}
	} else if (tokens[last_node_pos].type == TOKEN_WORD ||
			   tokens[last_node_pos].type == TOKEN_LPAREN) {
		cmd_suffix->op.word = calloc(1, sizeof(leaf_t));
		if (!cmd_suffix->op.word) {
			errno = ENOMEM;
			return DROP_ERR;
		}

		if (parse_leaf(cmd_suffix->op.word, &tokens[last_node_pos],
					   num_tokens - last_node_pos)) {
			cmd_suffix_deinit(cmd_suffix);
			return DROP_ERR;
		}
	} else {
		errno = EINVAL;
		return DROP_ERR;
	}

	if (last_node_pos > 0) {
		cmd_suffix->prefix = calloc(1, sizeof(cmd_suffix_t));
		if (!cmd_suffix->prefix) {
			errno = ENOMEM;
			cmd_suffix_deinit(cmd_suffix);
			return DROP_ERR;
		}

		// Parse prefixed cmd_suffix, cleanup if it fails
		if (parse_cmd_suffix((cmd_suffix_t *)cmd_suffix->prefix, tokens,
							 prefix_end_pos + 1) == DROP_ERR) {
			cmd_suffix_deinit(cmd_suffix);
			return DROP_ERR;
		}
	}

	return DROP_OK;
}

void simple_cmd_deinit(simple_cmd_t *simple_cmd) {
	if (!simple_cmd)
		return;

	leaf_deinit(simple_cmd->cmd_name);
	free(simple_cmd->cmd_name);
	cmd_suffix_deinit(simple_cmd->cmd_suffix);
	free(simple_cmd->cmd_suffix);
}

int parse_simple_cmd(simple_cmd_t *simple_cmd, token_t *tokens,
					 int num_tokens) {
  if (!simple_cmd || !tokens || !num_tokens) {
    errno = EINVAL;
    return DROP_ERR;
  }

  int suffix_pos;
  if (tokens->type == TOKEN_WORD) {
    suffix_pos = 1;
  }
  else if (tokens->type == TOKEN_LPAREN) {
    int right_paren = get_right_paren(tokens, num_tokens);
    if (right_paren == -1) {
      errno = EINVAL;
      return DROP_ERR;
    }

    suffix_pos = right_paren + 1;
  }
  else {
    errno = EINVAL;
    return DROP_ERR; 
  }
  
  simple_cmd->cmd_name = calloc(1, sizeof(leaf_t)); 
  if (!simple_cmd->cmd_name) {
    errno = ENOMEM;
    return DROP_ERR;
  }

  if (suffix_pos < num_tokens) {
    simple_cmd->cmd_suffix = calloc(1, sizeof(cmd_suffix_t));
    if (!simple_cmd->cmd_suffix) {
      errno = ENOMEM;
      simple_cmd_deinit(simple_cmd);
      return DROP_ERR;
    }

    if (parse_cmd_suffix(simple_cmd->cmd_suffix, &tokens[suffix_pos], num_tokens - suffix_pos) == DROP_ERR) {
      simple_cmd_deinit(simple_cmd);
      return DROP_ERR;
    }
  }
    
	return DROP_OK;
}

void pipe_seq_deinit(pipe_seq_t *pipe_seq) {
  if (!pipe_seq) return;

  simple_cmd_deinit(pipe_seq->simple_cmd);
  free(pipe_seq->simple_cmd);
  pipe_seq_deinit(pipe_seq->pipe_seq);
  free((pipe_seq_t*)pipe_seq->pipe_seq);
}

int parse_pipe_seq(pipe_seq_t *pipe_seq, token_t *tokens, int num_tokens) {
  if (!pipe_seq || !tokens || !num_tokens) {
    errno = EINVAL;
    return DROP_ERR;
  }

  int pipe_idx = -1;
  // last pipe cannot be index 0, has to be at least index 1
  for (int i = num_tokens - 1; i >= 1; --i) {
    if (tokens[i].type == TOKEN_RPAREN) {
      i = get_left_paren(tokens, i);
      if (i == -1) {
        errno = EINVAL;
        return DROP_ERR;
      }
    }
    else if (tokens[i].type == TOKEN_PIPE) {
      pipe_idx = i;
      break;
    }
  }

  if (pipe_idx > 0) {
    pipe_seq->pipe_seq = calloc(1, sizeof(pipe_seq_t));
    if (!pipe_seq->pipe_seq) {
      errno = ENOMEM;
      return DROP_ERR;
    }

    if (parse_pipe_seq(pipe_seq->pipe_seq, tokens, pipe_idx + 1) == DROP_ERR) {
      pipe_seq_deinit(pipe_seq);
      return DROP_ERR;
    }
  }

  pipe_seq->simple_cmd = calloc(1, sizeof(simple_cmd_t));
  if (!pipe_seq->simple_cmd) {
    errno = ENOMEM;
    pipe_seq_deinit(pipe_seq);
    return DROP_ERR;
  }

  if (parse_simple_cmd(pipe_seq->simple_cmd, &tokens[pipe_idx + 1], num_tokens - (pipe_idx + 1)) == DROP_ERR) {
    pipe_seq_deinit(pipe_seq);
    return DROP_ERR;
  }

	return DROP_OK;
}

void program_deinit(program_t *program) {
	if (!program)
		return;
	pipe_seq_deinit(program->pipe_seq);
	free(program->pipe_seq);
}

int parse_program(program_t *program, token_t *tokens, int num_tokens) {
  if (!program || !tokens || !num_tokens) {
    errno = EINVAL;
    return DROP_ERR;
  }

  program->pipe_seq = calloc(1, sizeof(pipe_seq_t));
  if (!program->pipe_seq) {
    errno = ENOMEM;
    return DROP_ERR;
  }

  if (parse_pipe_seq(program->pipe_seq, tokens, num_tokens) == DROP_ERR) {
    program_deinit(program);
    return DROP_ERR;
  }
	return DROP_OK;
}

