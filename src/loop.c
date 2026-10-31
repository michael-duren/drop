#include "drop.h"
#include "eval.h"
#include "parser.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RL_BUFSIZE 1024

char *drop_read_line(void) {
	size_t bufsize = RL_BUFSIZE;
	size_t position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);

	if (!buffer) {
		fprintf(stderr, "drop: allocation error\n");
		exit(EXIT_FAILURE);
	}

	int c;
	while (true) {
		c = getchar();
		if (c == EOF && position == 0) {
			free(buffer);
			return NULL;
		}

		if (c == EOF || c == '\n') {
			buffer[position] = '\n';
			position++;
			// terminate
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;

		// leave space for 2 for \n and \0
		if (position >= bufsize - 2) {
			bufsize *= 2;
			char *grown = realloc(buffer, bufsize);
			if (!grown) {
				free(buffer);
				fprintf(stderr, "drop: allocation error\n");
				exit(EXIT_FAILURE);
			}
			buffer = grown;
		}
	}
}

// first shell loop
// placeholder for when we have actual logic
void shell_loop(void) {
	while (true) {
		char *line;

		printf("> ");
		fflush(stdout);
		line = drop_read_line();
		// Ctrl-D on an empty line
		if (line == NULL) {
			putchar('\n');
			break;
		}

		int rc = eval_line(line);
		if (rc == DROP_EXIT) {
			break;
		}

		free(line);
	}
}
