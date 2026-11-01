CC     := cc
# -fsanitize: turn on ASan + UBSan
# -fno-sanitize-recover=all: stop the program on any UBSan error
# -fno-omit-frame-pointer: cleaner stack traces in reports
CFLAGS := -std=c23 -Wall -Wextra -Werror -pedantic -g -I../include \
		  -fsanitize=address,undefined -fno-sanitize-recover=all \
		  -fno-omit-frame-pointer

BUILD  := ../build/tests

# Each test is tests/<name>_test.c linked against the src/ files it needs.
TESTS  := add_test scanner_test builtins_test parser_test

add_test_SRCS := ../src/add.c
builtins_test_SRCS := ../src/builtins.c
scanner_test_SRCS := ../src/scanner.c ../src/token.c ../include/scanner.h ../include/token.h
parser_test_SRCS := ../src/parser.c $(scanner_test_SRCS) ../include/parser.h

.PHONY: all build clean $(TESTS)

all: $(TESTS)

# Compile every test without running it.
build: $(addprefix $(BUILD)/,$(TESTS))

# `make add_test` builds and runs a single test.
$(TESTS): %: $(BUILD)/%
	$<

$(BUILD)/add_test: add_test.c $(add_test_SRCS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/scanner_test: scanner_test.c $(scanner_test_SRCS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

$(BUILD)/parser_test: parser_test.c  $(parser_test_SRCS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

$(BUILD)/builtins_test: builtins_test.c $(builtins_test_SRCS) ../include/builtins.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
