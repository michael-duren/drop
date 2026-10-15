CC      := cc
CFLAGS  := -std=c23 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -pedantic -g -Iinclude
LDFLAGS :=

BUILD   := build
BIN     := $(BUILD)/drop

SRCS    := $(wildcard src/*.c)
OBJS    := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))

.PHONY: all clean run test

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD):
	mkdir -p $@

# `make run ARGS="..."` builds if needed, then runs the binary.
run: $(BIN)
	./$(BIN) $(ARGS)

test:
	$(MAKE) -C tests all

clean:
	rm -rf $(BUILD)
	$(MAKE) -C tests clean

-include $(OBJS:.o=.d)
