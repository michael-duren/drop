CC     := cc
CFLAGS := -std=c23 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -g -I../include

BUILD  := ../build/tests

# Each test is tests/<name>_test.c linked against the src/ files it needs.
TESTS  := add_test

add_test_SRCS := ../src/add.c

.PHONY: all clean $(TESTS)

all: $(TESTS)

# `make add_test` builds and runs a single test.
$(TESTS): %: $(BUILD)/%
	$<

$(BUILD)/add_test: add_test.c $(add_test_SRCS) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD):
	mkdir -p $@

clean:
	rm -rf $(BUILD)
