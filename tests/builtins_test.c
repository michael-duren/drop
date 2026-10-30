#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"

// asserts the process is currently in want
static void expect_cwd(const char *want) {
	char *cwd = getcwd(NULL, 0);
	assert(cwd != NULL);
	assert(strcmp(cwd, want) == 0);
	free(cwd);
}

// cd to a real directory succeeds and moves us there
static void test_cd_path(void) {
	char *argv[] = {"cd", "/"};
	assert(drop_cd(2, argv) == 0);
	expect_cwd("/");
}

// cd to a directory that doesn't exist fails
static void test_cd_missing_dir(void) {
	char *start[] = {"cd", "/"};
	assert(drop_cd(2, start) == 0);

	char *argv[] = {"cd", "/nope"};
	assert(drop_cd(2, argv) == 1);
	// a failed cd must leave us where we were
	expect_cwd("/");
}

// bare cd goes to $HOME
static void test_cd_home(void) {
	assert(setenv("HOME", "/tmp", 1) == 0);
	char *argv[] = {"cd"};
	assert(drop_cd(1, argv) == 0);
	expect_cwd("/tmp");
}

// bare cd fails when $HOME isn't set
static void test_cd_home_unset(void) {
	assert(unsetenv("HOME") == 0);
	char *argv[] = {"cd"};
	assert(drop_cd(1, argv) == 1);
}

// cd rejects more than one argument
static void test_cd_too_many_args(void) {
	char *argv[] = {"cd", "a", "b"};
	assert(drop_cd(3, argv) == 1);
}

// pwd with no arguments succeeds
static void test_pwd(void) {
	char *argv[] = {"pwd"};
	assert(drop_pwd(1, argv) == 0);
}

// pwd rejects any arguments
static void test_pwd_too_many_args(void) {
	char *argv[] = {"pwd", "extra"};
	assert(drop_pwd(2, argv) == 1);
}

// help with no arguments succeeds
static void test_help(void) {
	char *argv[] = {"help"};
	assert(drop_help(1, argv) == 0);
}

// help rejects any arguments
static void test_help_too_many_args(void) {
	char *argv[] = {"help", "extra"};
	assert(drop_help(2, argv) == 1);
}

#define RUN(t)                                                                 \
	do {                                                                       \
		printf("  %s\n", #t);                                                  \
		fflush(stdout);                                                        \
		t();                                                                   \
	} while (0)

int main(void) {
	RUN(test_cd_path);
	RUN(test_cd_missing_dir);
	RUN(test_cd_home);
	RUN(test_cd_home_unset);
	RUN(test_cd_too_many_args);
	RUN(test_pwd);
	RUN(test_pwd_too_many_args);
	RUN(test_help);
	RUN(test_help_too_many_args);

	printf("builtins_test: ok\n");
	return 0;
}
