#include "drop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RL_BUFSIZE 1024

// first shell loop
// placeholder for when we have actual logic
void shell_loop(void) {
	while (1) {
		char *line;
		size_t len = 0;
		ssize_t read;

		printf("> ");
		read = getline(&line, &len, stdin);

		if (read == DROP_ERR) {
			puts("unable to read line");
			exit(EXIT_FAILURE);
		}

		if (strcmp(line, "exit\n") == 0) {
			puts("goodbye!");
			exit(0);
		}
		printf("you wrote: %s", line);

		free(line);
	}
}
