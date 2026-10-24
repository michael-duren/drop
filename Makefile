CC      := cc
CFLAGS  := -std=c23 -Wall -Wextra -Werror -pedantic -g -Iinclude
LDFLAGS :=

BUILD   := build
BIN     := $(BUILD)/drop

SRCS    := $(wildcard src/*.c)
OBJS    := $(patsubst src/%.c,$(BUILD)/%.o,$(SRCS))
FMT_SRCS := $(wildcard src/*.c include/*.h tests/*.c)

.PHONY: all build clean run test test-valgrind fmt fmt-check bear

all: build

build: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

# Create the output dir inline: a `build:` dir rule would collide with the
# phony `build` target above.
$(BUILD)/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

# `make run ARGS="..."` builds if needed, then runs the binary.
run: $(BIN)
	./$(BIN) $(ARGS)

test:
	$(MAKE) -C tests all

# Runs the binary under Valgrind. Catches uninitialized reads, leaks,
# and use-after-free that ASan (in the tests build) does not.
# Use: make test-valgrind ARGS="some input"
test-valgrind: build
	valgrind --leak-check=full --error-exitcode=1 ./$(BIN) $(ARGS)

# Rewrites files in place using .clang-format (LLVM style if none exists).
fmt:
	clang-format -i $(FMT_SRCS)

# Fails if any file differs from what `make fmt` would produce. Used by CI.
fmt-check:
	clang-format --dry-run --Werror $(FMT_SRCS)

# Regenerate compile_commands.json for clangd. -B forces every compile to
# rerun so bear sees all of them. Tests are compiled, not run: ASan breaks
# under bear's LD_PRELOAD.
bear:
	bear -- $(MAKE) -B build
	bear --append -- $(MAKE) -B -C tests build

clean:
	rm -rf $(BUILD)
	$(MAKE) -C tests clean

-include $(OBJS:.o=.d)
