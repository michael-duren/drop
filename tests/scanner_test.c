#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "drop.h"
#include "scanner.h"

typedef struct {
	token_type_t type;
	const char *lexeme;
} expected_t;

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

// Scans src and asserts the token stream matches want exactly.
static void expect_tokens(const char *src, const expected_t *want,
						  size_t want_len) {
	// scanner takes a mutable char *, so copy out of the literal
	char buf[256];
	assert(strlen(src) < sizeof(buf));
	strcpy(buf, src);

	scanner_t s;
	assert(scanner_init(&s, buf) == DROP_OK);
	assert(scan_tokens(&s) == DROP_OK);

	assert(s.tokens.len == want_len);
	for (size_t i = 0; i < want_len; i++) {
		const token_t *tok = &s.tokens.tokens[i];
		assert(tok->type == want[i].type);
		assert(tok->len == strlen(want[i].lexeme));
		assert(strncmp(tok->start, want[i].lexeme, tok->len) == 0);
	}

	scanner_deinit(&s);
}

static void expect_error(const char *src) {
	char buf[256];
	assert(strlen(src) < sizeof(buf));
	strcpy(buf, src);

	scanner_t s;
	assert(scanner_init(&s, buf) == DROP_OK);
	assert(scan_tokens(&s) == DROP_ERR);

	scanner_deinit(&s);
}

static void test_pipeline(void) {
	expected_t want[] = {
		{TOKEN_WORD, "ls"},	  {TOKEN_WORD, "-la"}, {TOKEN_PIPE, "|"},
		{TOKEN_WORD, "grep"}, {TOKEN_WORD, "-i"},  {TOKEN_WORD, "foo"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("ls -la | grep -i foo", want, LEN(want));
}

static void test_empty(void) {
	expected_t want[] = {{TOKEN_EOF, ""}};
	expect_tokens("", want, LEN(want));
}

static void test_whitespace(void) {
	expected_t want[] = {
		{TOKEN_WORD, "ls"},
		{TOKEN_WORD, "-l"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("  ls \t -l  ", want, LEN(want));
}

static void test_trailing_newline(void) {
	// drop_read_line always appends '\n'
	expected_t want[] = {
		{TOKEN_WORD, "ls"},
		{TOKEN_NEWLINE, "\n"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("ls\n", want, LEN(want));
}

static void test_redirects(void) {
	expected_t want[] = {
		{TOKEN_WORD, "cat"}, {TOKEN_LESS, "<"},	  {TOKEN_WORD, "in"},
		{TOKEN_GREAT, ">"},	 {TOKEN_WORD, "out"}, {TOKEN_EOF, ""},
	};
	expect_tokens("cat < in > out", want, LEN(want));
}

static void test_operators_no_spaces(void) {
	expected_t want[] = {
		{TOKEN_WORD, "ls"}, {TOKEN_PIPE, "|"},	 {TOKEN_WORD, "wc"},
		{TOKEN_GREAT, ">"}, {TOKEN_WORD, "out"}, {TOKEN_EOF, ""},
	};
	expect_tokens("ls|wc>out", want, LEN(want));
}

static void test_parens(void) {
	expected_t want[] = {
		{TOKEN_LPAREN, "("},
		{TOKEN_WORD, "ls"},
		{TOKEN_RPAREN, ")"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("(ls)", want, LEN(want));
}

static void test_string(void) {
	expected_t want[] = {
		{TOKEN_WORD, "echo"},
		{TOKEN_STRING, "\"hello world\""},
		{TOKEN_EOF, ""},
	};
	expect_tokens("echo \"hello world\"", want, LEN(want));
}

static void test_unterminated_string(void) { expect_error("echo \"abc"); }

static void test_word_chars(void) {
	expected_t want[] = {
		{TOKEN_WORD, "echo"},
		{TOKEN_WORD, "$HOME"},
		{TOKEN_WORD, "./foo.txt"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("echo $HOME ./foo.txt", want, LEN(want));
}

static void test_many_tokens(void) {
	// more than TOKEN_ARR_INIT_SIZE (8) so the token array has to grow
	expected_t want[] = {
		{TOKEN_WORD, "a"}, {TOKEN_WORD, "b"}, {TOKEN_WORD, "c"},
		{TOKEN_WORD, "d"}, {TOKEN_WORD, "e"}, {TOKEN_WORD, "f"},
		{TOKEN_WORD, "g"}, {TOKEN_WORD, "h"}, {TOKEN_WORD, "i"},
		{TOKEN_WORD, "j"}, {TOKEN_WORD, "k"}, {TOKEN_WORD, "l"},
		{TOKEN_WORD, "m"}, {TOKEN_WORD, "n"}, {TOKEN_WORD, "o"},
		{TOKEN_WORD, "p"}, {TOKEN_WORD, "q"}, {TOKEN_WORD, "r"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("a b c d e f g h i j k l m n o p q r", want, LEN(want));
}

#define RUN(t)                                                                 \
	do {                                                                       \
		printf("  %s\n", #t);                                                  \
		fflush(stdout);                                                        \
		t();                                                                   \
	} while (0)

int main(void) {
	RUN(test_pipeline);
	RUN(test_empty);
	RUN(test_whitespace);
	RUN(test_trailing_newline);
	RUN(test_redirects);
	RUN(test_operators_no_spaces);
	RUN(test_parens);
	RUN(test_string);
	RUN(test_unterminated_string);
	RUN(test_word_chars);
	RUN(test_many_tokens);

	printf("scanner_test: ok\n");
	return 0;
}
