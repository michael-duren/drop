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

int get_left_paren(token_t *tokens, int r_idx) {
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
	(void)simple_cmd;
	(void)tokens;
	(void)num_tokens;
	return DROP_OK;
}

int parse_pipe_seq(pipe_seq_t *pipe_seq, token_t *tokens, int num_tokens) {
	(void)pipe_seq;
	(void)tokens;
	(void)num_tokens;
	return DROP_OK;
}

void pipe_seq_deinit(pipe_seq_t *pipe_seq) { (void)pipe_seq; }

int parse_program(program_t *program, token_t *tokens, int num_tokens) {
	(void)program;
	(void)tokens;
	(void)num_tokens;
	return DROP_OK;
}

void program_deinit(program_t *program) {
	if (!program)
		return;
	pipe_seq_deinit(program->pipe_seq);
	free(program->pipe_seq);
}
