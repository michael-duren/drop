#include <assert.h>
#include <errno.h>
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

static void test_many_grows(void) {
	// 40 words forces several reallocs, not just one
	char src[128] = "";
	expected_t want[41];
	for (size_t i = 0; i < 40; i++) {
		strcat(src, "a ");
		want[i] = (expected_t){TOKEN_WORD, "a"};
	}
	want[40] = (expected_t){TOKEN_EOF, ""};
	expect_tokens(src, want, LEN(want));
}

static void test_null_args(void) {
	char buf[] = "ls";
	scanner_t s;

	errno = 0;
	assert(scanner_init(NULL, buf) == DROP_ERR);
	assert(errno == EINVAL);

	errno = 0;
	assert(scanner_init(&s, NULL) == DROP_ERR);
	assert(errno == EINVAL);

	errno = 0;
	assert(scan_tokens(NULL) == DROP_ERR);
	assert(errno == EINVAL);
}

static void test_invalid_word_char(void) { expect_error("ls a@b"); }

static void test_unsupported_chars(void) {
	expect_error("a;b");
	expect_error("a&b");
	// TODO: we should probably handle this, single quotess should be fine
	expect_error("'x'");
}

static void test_lone_quote(void) { expect_error("\""); }

// NOTE: maybe we want to hanlde UTF-8 at some point
static void test_non_ascii(void) { expect_error("caf\xc3\xa9"); }

static void test_empty_string(void) {
	expected_t want[] = {
		{TOKEN_STRING, "\"\""},
		{TOKEN_EOF, ""},
	};
	expect_tokens("\"\"", want, LEN(want));
}

static void test_string_with_operators(void) {
	expected_t want[] = {
		{TOKEN_STRING, "\"a | b > c\""},
		{TOKEN_EOF, ""},
	};
	expect_tokens("\"a | b > c\"", want, LEN(want));
}

static void test_string_with_newline(void) {
	expected_t want[] = {
		{TOKEN_STRING, "\"a\nb\""},
		{TOKEN_EOF, ""},
	};
	expect_tokens("\"a\nb\"", want, LEN(want));
}

static void test_string_word_adjacent(void) {
	// a word directly after a string is split off, but a quote directly
	// after a word is rejected
	expected_t want[] = {
		{TOKEN_STRING, "\"a\""},
		{TOKEN_WORD, "b"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("\"a\"b", want, LEN(want));
	expect_error("echo\"hi\"");
}

static void test_whitespace_only(void) {
	expected_t want[] = {{TOKEN_EOF, ""}};
	expect_tokens("   \t ", want, LEN(want));
}

static void test_newline_only(void) {
	expected_t want[] = {
		{TOKEN_NEWLINE, "\n"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("\n", want, LEN(want));
}

static void test_multiple_lines(void) {
	expected_t want[] = {
		{TOKEN_WORD, "a"},	   {TOKEN_NEWLINE, "\n"}, {TOKEN_WORD, "b"},
		{TOKEN_NEWLINE, "\n"}, {TOKEN_EOF, ""},
	};
	expect_tokens("a\nb\n", want, LEN(want));
}

static void test_blank_lines(void) {
	expected_t want[] = {
		{TOKEN_NEWLINE, "\n"},
		{TOKEN_NEWLINE, "\n"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("\n\n", want, LEN(want));
}

// static void test_doubled_operators(void) {
// 	// no >>, || or <> tokens yet, so each char scans on its own. update
// 	// when TOKEN_DGREAT / TOKEN_OR_IF land
// 	expected_t dgreat[] = {
// 		{TOKEN_GREAT, ">"},
// 		{TOKEN_GREAT, ">"},
// 		{TOKEN_EOF, ""},
// 	};
// 	expect_tokens(">>", dgreat, LEN(dgreat));
//
// 	expected_t or_if[] = {
// 		{TOKEN_PIPE, "|"},
// 		{TOKEN_PIPE, "|"},
// 		{TOKEN_EOF, ""},
// 	};
// 	expect_tokens("||", or_if, LEN(or_if));
//
// 	expected_t lessgreat[] = {
// 		{TOKEN_LESS, "<"},
// 		{TOKEN_GREAT, ">"},
// 		{TOKEN_EOF, ""},
// 	};
// 	expect_tokens("<>", lessgreat, LEN(lessgreat));
// }

static void test_operator_at_edges(void) {
	expected_t leading[] = {
		{TOKEN_PIPE, "|"},
		{TOKEN_WORD, "ls"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("| ls", leading, LEN(leading));

	expected_t trailing[] = {
		{TOKEN_WORD, "ls"},
		{TOKEN_PIPE, "|"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("ls |", trailing, LEN(trailing));
}

static void test_nested_parens(void) {
	expected_t pipe[] = {
		{TOKEN_LPAREN, "("}, {TOKEN_WORD, "ls"},  {TOKEN_PIPE, "|"},
		{TOKEN_WORD, "wc"},	 {TOKEN_RPAREN, ")"}, {TOKEN_EOF, ""},
	};
	expect_tokens("(ls|wc)", pipe, LEN(pipe));

	expected_t nested[] = {
		{TOKEN_LPAREN, "("}, {TOKEN_LPAREN, "("}, {TOKEN_WORD, "a"},
		{TOKEN_RPAREN, ")"}, {TOKEN_RPAREN, ")"}, {TOKEN_EOF, ""},
	};
	expect_tokens("((a))", nested, LEN(nested));
}

static void test_fd_redirect(void) {
	// no TOKEN_IO_NUMBER yet, so the 2 is a plain word
	expected_t want[] = {
		{TOKEN_WORD, "2"},
		{TOKEN_GREAT, ">"},
		{TOKEN_WORD, "f"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("2>f", want, LEN(want));
}

static void test_word_underscore_digits(void) {
	expected_t want[] = {
		{TOKEN_WORD, "foo_bar2"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("foo_bar2", want, LEN(want));
}

static void test_punctuation_words(void) {
	expected_t want[] = {
		{TOKEN_WORD, ".."},
		{TOKEN_WORD, "-"},
		{TOKEN_WORD, "/"},
		{TOKEN_EOF, ""},
	};
	expect_tokens(".. - /", want, LEN(want));
}

static void test_invalid_first_char(void) {
	// add_token only validates chars after the first one
	expect_error("@foo");
	expect_error("# comment");
	expect_error("ls *.c");
}

static void test_carriage_return(void) {
	// \r should be skipped like ' ' and '\t', not scanned as a word
	expected_t want[] = {
		{TOKEN_WORD, "ls"},
		{TOKEN_EOF, ""},
	};
	expect_tokens("ls\r", want, LEN(want));
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
	RUN(test_many_grows);
	RUN(test_null_args);
	RUN(test_invalid_word_char);
	RUN(test_unsupported_chars);
	RUN(test_lone_quote);
	RUN(test_non_ascii);
	RUN(test_empty_string);
	RUN(test_string_with_operators);
	RUN(test_string_with_newline);
	RUN(test_string_word_adjacent);
	RUN(test_whitespace_only);
	RUN(test_newline_only);
	RUN(test_multiple_lines);
	RUN(test_blank_lines);
	RUN(test_doubled_operators);
	RUN(test_operator_at_edges);
	RUN(test_nested_parens);
	RUN(test_fd_redirect);
	RUN(test_word_underscore_digits);
	RUN(test_punctuation_words);
	RUN(test_eof_points_at_end);
	// known bugs: these fail until scanner.c is fixed
	RUN(test_invalid_first_char);
	RUN(test_carriage_return);

	printf("scanner_test: ok\n");
	return 0;
}
