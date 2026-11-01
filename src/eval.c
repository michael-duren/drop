#include "eval.h"
#include "drop.h"
#include "parser.h"
#include "scanner.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int eval_program(program_t *p);

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
		perror("parse_program");
		return rc;
	}
	eval_program(&p);

	scanner_deinit(&s);
	program_deinit(&p);

	if (strcmp(line, "exit\n") == 0) {
		puts("goodbye!");
		free(line);
		return DROP_EXIT;
	}
	return 0;
}

int eval_simple_cmd(simple_cmd_t *s) {
	if (!s) {
		// TODO: revist if we want to return error here
		return DROP_OK;
	}

	int pid = fork();
	int wstatus;
	switch (pid) {
	case -1:
		perror("fork");
		break;
	case 0: {
		// TODO: handle this better
		char *cmd_name = s->cmd_name->val.literal;
		if (s->cmd_suffix != NULL) {
			char *const args[] = {cmd_name, s->cmd_suffix->op.word->val.literal,
								  NULL};
			execvp(s->cmd_name->val.literal, args);
		} else {
			char *const args[] = {cmd_name, NULL};
			execvp(s->cmd_name->val.literal, args);
		}
		break;
	}
	default: {
		wait(&wstatus);
		return WEXITSTATUS(wstatus);
	}
	}
	// should not get to this line
	return DROP_ERR;
}

int eval_pipe_seq(pipe_seq_t *ps) {
	if (!ps) {
		return DROP_OK;
	}
	if (ps->pipe_seq != NULL) {
		// TODO: pass error codes
		eval_pipe_seq(ps);
	}
	return eval_simple_cmd(ps->simple_cmd);
}

int eval_program(program_t *p) {
	if (!p) {
		errno = EINVAL;
		return DROP_ERR;
	}

	eval_pipe_seq(p->pipe_seq);
	return DROP_OK;
}
