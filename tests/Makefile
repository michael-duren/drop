CC     := cc
CFLAGS := -std=c23 -Wall -Wextra -Werror -pedantic -g -I../include

BUILD  := ../build/tests

# Each test is tests/<name>_test.c linked against the src/ files it needs.
TESTS  := add_test scanner_test

add_test_SRCS := ../src/add.c
scanner_test_SRCS := ../src/scanner.c ../src/token.c

.PHONY: all clean $(TESTS)

all: $(TESTS)

# `make add_test` builds and runs a single test.
$(TESTS): %: $(BUILD)/%
	$<

$(BUILD)/add_test: add_test.c $(add_test_SRCS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/scanner_test: scanner_test.c $(scanner_test_SRCS) ../include/scanner.h ../include/token.h | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $(filter %.c,$^)

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
