#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "drop.h"
#include "parser.h"
#include "token.h"

struct layer_node {
  char *str;
  struct layer_node *last;
};

enum node_type {
  T_LEAF,
  T_IO_REDIR,
  T_CMD_SUFFIX,
  T_SIMPLE_CMD,
  T_PIPE_SEQ,
  T_PROGRAM,
};

typedef union {
  program_t *program;
  pipe_seq_t *pipe_seq;
  simple_cmd_t *simple_cmd;
  cmd_suffix_t *cmd_suffix;
  io_redir_t *io_redir;
  leaf_t *leaf;
} prsr_node;

struct prsr_test_meta {
  enum node_type type;
  prsr_node src;
  prsr_node want;

  struct layer_node *layer_head; 
};
// static void expect_tree(struct prsr_test_meta *meta);
static void expect_program(struct prsr_test_meta *meta);
static void print_layers(struct layer_node *layer_head);

#define prsr_assert(cond, lh) do { \
  if (!(cond)) { \
    fprintf(stderr, "Assertion failed: %s\nLine: %d\n", #cond, __LINE__); \
    print_layers(lh); \
    abort(); \
  } \
} while (0)

static void add_layer_string(struct prsr_test_meta *meta, char *layer_str) {
  struct layer_node *new_layer = calloc(1, sizeof(struct layer_node));
  new_layer->str = layer_str;
  struct layer_node **old_layer = &(meta->layer_head);
  new_layer->last = *old_layer;
}

static void print_layers(struct layer_node *layer_head) {
  do {
    printf("-> %s\n", layer_head->str);
  } while (layer_head->last);
}

static void expect_leaf(struct prsr_test_meta *meta) {
  leaf_t *src = meta->src.leaf;
  leaf_t *want = meta->want.leaf;

  char *layer_str = malloc(sizeof(char) * 5);
  layer_str = "leaf\0";
  add_layer_string(meta, layer_str);

  prsr_assert(src->is_subcmd == want->is_subcmd, meta->layer_head);

  if (src->is_subcmd) {
    prsr_assert(src->val.program != NULL, meta->layer_head);

    struct prsr_test_meta *nmeta = meta;
    nmeta->type = T_PROGRAM;
    nmeta->src.program = src->val.program;
    nmeta->want.program = want->val.program;
    expect_program(nmeta);
  } 
  else {
    prsr_assert(src->val.literal != NULL, meta->layer_head);
    prsr_assert(src->val.literal == want->val.literal, meta->layer_head);
  }
}

static void expect_io_redir(struct prsr_test_meta *meta) {
  io_redir_t *src = meta->src.io_redir;
  io_redir_t *want = meta->want.io_redir;

  char *layer_str = malloc(sizeof(char) * 9);
  layer_str = "io_redir\0";
  add_layer_string(meta, layer_str);

  prsr_assert(src->op == want->op, meta->layer_head);
  prsr_assert(src->filename != NULL, meta->layer_head);

  struct prsr_test_meta *nmeta = meta;
  nmeta->type = T_LEAF;
  nmeta->src.leaf = src->filename;
  nmeta->want.leaf = want->filename;
  expect_leaf(nmeta);
}

static void expect_cmd_suffix(struct prsr_test_meta *meta) {
  cmd_suffix_t *src = meta->src.cmd_suffix;
  cmd_suffix_t *want = meta->want.cmd_suffix;

  char *layer_str = malloc(sizeof(char) * 11);
  layer_str = "cmd_suffix\0";
  add_layer_string(meta, layer_str);

  prsr_assert(src->is_redir == want->is_redir, meta->layer_head);
  if (src->is_redir) {
    prsr_assert(src->op.io_redir != NULL, meta->layer_head);

    struct prsr_test_meta *nmeta = meta;
    nmeta->type = T_IO_REDIR;
    nmeta->src.io_redir = src->op.io_redir;
    nmeta->want.io_redir = want->op.io_redir;
    expect_io_redir(nmeta);
  }
  else {
    prsr_assert(src->op.word != NULL, meta->layer_head);
    struct prsr_test_meta *nmeta = meta;
    nmeta->type = T_LEAF;
    nmeta->src.leaf = src->op.word;
    nmeta->want.leaf = want->op.word;
    expect_leaf(nmeta);
  }

  if (want->prefix == NULL) {
    prsr_assert(src->prefix == NULL, meta->layer_head);
  }
  else {
    prsr_assert(src->prefix != NULL, meta->layer_head);
    struct prsr_test_meta *pxmeta = meta;
    pxmeta->type = T_CMD_SUFFIX;
    pxmeta->src.cmd_suffix = (cmd_suffix_t*)src->prefix;
    pxmeta->want.cmd_suffix = (cmd_suffix_t*)want->prefix;
    expect_cmd_suffix(pxmeta);
  }
}

static void expect_simple_cmd(struct prsr_test_meta *meta) {
  simple_cmd_t *src = meta->src.simple_cmd;
  simple_cmd_t *want = meta->want.simple_cmd;

  char *layer_str = malloc(sizeof(char) * 11);
  layer_str = "simple_cmd\0";
  add_layer_string(meta, layer_str);

  prsr_assert(src->cmd_name != NULL, meta->layer_head);

  // Validate cmd name as leaf node
  struct prsr_test_meta *nmeta = meta;
  nmeta->type = T_LEAF;
  nmeta->src.leaf = src->cmd_name;
  nmeta->want.leaf = want->cmd_name;
  expect_leaf(nmeta);

  // Validate cmd suffix if exists
  if (want->cmd_suffix == NULL) {
    prsr_assert(src->cmd_suffix == NULL, meta->layer_head);
  }
  else {
    prsr_assert(src->cmd_suffix != NULL, meta->layer_head);
    struct prsr_test_meta *sxmeta = meta;
    sxmeta->type = T_CMD_SUFFIX;
    sxmeta->src.cmd_suffix = src->cmd_suffix;
    sxmeta->want.cmd_suffix = want->cmd_suffix;
    expect_cmd_suffix(sxmeta);
  }
}


static void expect_pipe_seq(struct prsr_test_meta *meta) {
  pipe_seq_t *src = meta->src.pipe_seq;
  pipe_seq_t *want = meta->want.pipe_seq;

  char *layer_str = malloc(sizeof(char) * 9);
  layer_str = "pipe_seq\0";
  add_layer_string(meta, layer_str);

  prsr_assert(src->simple_cmd != NULL, meta->layer_head);

  struct prsr_test_meta *nmeta = meta;
  nmeta->type = T_SIMPLE_CMD;
  nmeta->src.simple_cmd = src->simple_cmd;
  nmeta->want.simple_cmd = want->simple_cmd;
  expect_simple_cmd(nmeta);

  if (want->pipe_seq == NULL) {
    prsr_assert(src->pipe_seq == NULL, meta->layer_head);
  }
  else {
    prsr_assert(src->pipe_seq != NULL, meta->layer_head);
    struct prsr_test_meta *psmeta = meta;
    psmeta->type = T_PIPE_SEQ;
    psmeta->src.pipe_seq = src->pipe_seq;
    psmeta->want.pipe_seq = want->pipe_seq;
    expect_pipe_seq(psmeta);
  }
}

static void expect_program(struct prsr_test_meta *meta) {
  program_t *src = meta->src.program;
  program_t *want = meta->want.program;

  char *layer_str = malloc(sizeof(char) * 8);
  layer_str = "program\0";
  add_layer_string(meta, layer_str);

  prsr_assert(src->pipe_seq != NULL, meta->layer_head);

  struct prsr_test_meta *nmeta = meta;
  nmeta->type = T_PIPE_SEQ;
  nmeta->src.pipe_seq = src->pipe_seq;
  nmeta->want.pipe_seq = want->pipe_seq;
  expect_pipe_seq(meta);
}

static void run_test(token_arr *src, program_t *want) {
  program_t *program = calloc(1, sizeof(program_t));

  assert(parse_program(program, src->tokens, src->len) != DROP_ERR);

  struct prsr_test_meta *meta = calloc(1, sizeof(struct prsr_test_meta));
  meta->type = T_PROGRAM;
  meta->src.program = program;
  meta->want.program = want;

  expect_program(meta);
}

