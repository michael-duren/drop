CC      := cc
CFLAGS  := -std=c23 -Wall -Wextra -Werror -pedantic -g -Iinclude
LDFLAGS :=

BUILD   := build
BIN     := $(BUILD)/drop

SRCS    := $(wildcard src/*.c)
OBJS    := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))

.PHONY: all clean test

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

$(BUILD):
	mkdir -p $@

test:
	$(MAKE) -C tests all

clean:
	rm -rf $(BUILD)
	$(MAKE) -C tests clean

-include $(OBJS:.o=.d)
