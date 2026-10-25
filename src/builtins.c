#include "builtins.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int drop_cd(int argc, char **argv) {
	if (argc > 2) {
		fprintf(stderr, "drop: cd: too many arguments\n");
		return 1;
	}

	char *path;
	// bare `cd` goes home, like bash
	if (argc == 1) {
		path = getenv("HOME");
		if (path == NULL) {
			fprintf(stderr, "drop: cd: HOME not set\n");
			return 1;
		}
	} else {
		path = argv[1];
	}

	if (chdir(path) == -1) {
		fprintf(stderr, "drop: cd: %s: %s\n", path, strerror(errno));
		return 1;
	}

	return 0;
}

int drop_pwd(int argc, char **argv) {
	(void)argv;
	if (argc > 1) {
		fprintf(stderr, "drop: pwd: too many arguments\n");
		return 1;
	}

	// passing NULL and 0 makes getcwd malloc a buffer of the right size,
	// then we free it when we're done
	char *cwd = getcwd(NULL, 0);
	if (cwd == NULL) {
		fprintf(stderr, "drop: pwd: %s\n", strerror(errno));
		return 1;
	}

	printf("%s\n", cwd);
	free(cwd);
	return 0;
}

int drop_help(int argc, char **argv) {
	(void)argv;
	if (argc > 1) {
		fprintf(stderr, "drop: help: too many arguments\n");
		return 1;
	}

	printf("These commands are built into the shell:\n"
		   " cd    change the current directory\n"
		   " help  show this message\n"
		   " pwd   print the current directory\n");
	return 0;
}
