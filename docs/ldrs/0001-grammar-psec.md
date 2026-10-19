# LDR on starting design

## Context

Define the strategy and beginning requirements for parsing user input in the drop shell.

## Scanner

Should handle all tokens initially warning about unsupported tokens but still creating them.
See the grammar below for initial features.

## Parser

We need to define our grammar using EBNF or start with a simplified version of the POSIX standard.
The POSIX grammar can be found [here](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html).

### Simplified grammar

Subset of the POSIX grammar covering simple commands, args, redirects, and pipes.
Rule names match POSIX so we can grow back toward the full grammar.

Symbols in yacc grammar:

- Definition: `:` defines a rule (e.g., `name : WORD`).
- Concatenation: implicit via adjacency; symbols must appear in order (e.g., `'>' filename`).
- Alternation: `|` at the start of an alternative selects one option (e.g., `A | B`).
- Optional: an empty alternative matches zero tokens (e.g., `x : A | /* empty */`).
- Repetition: no operator; use recursion with a base case (e.g., `list : WORD | list WORD` is one or more).
- Grouping: no parentheses; factor into a helper rule.
- Terminals: tokens from the scanner, UPPERCASE (`WORD`) or quoted literals (`'|'`).
- Nonterminals: lowercase names defined by other rules.
- Precedence: set by nesting, not textual order; deeper rules bind tighter.
- Comment: `/* ... */`.
- Termination: `;` ends a rule.

```
program        : pipe_sequence NEWLINE
               | NEWLINE                    /* empty line */
               ;
pipe_sequence  : simple_command
               | pipe_sequence '|' simple_command
               ;
simple_command : cmd_name cmd_suffix
               | cmd_name
               ;
cmd_name       : WORD
               ;
cmd_suffix     : WORD
               | io_redirect
               | cmd_suffix WORD
               | cmd_suffix io_redirect
               ;
io_redirect    : '<'    filename
               | '>'    filename
               | DGREAT filename            /* >> */
               ;
filename       : WORD
               ;
```

### Examples

#### `ls -la`

Tokens: `WORD(ls) WORD(-la) NEWLINE`

```
program
├── pipe_sequence
│   └── simple_command
│       ├── cmd_name ── WORD(ls)
│       └── cmd_suffix ── WORD(-la)
└── NEWLINE
```

#### `ls -la | grep -i "foo" | sort | uniq`

Tokens: `WORD(ls) WORD(-la) '|' WORD(grep) WORD(-i) STRING("foo") '|' WORD(sort) '|' WORD(uniq) NEWLINE`

`pipe_sequence` is left recursive, so each `|` wraps everything to its left:
`((ls -la | grep -i "foo") | sort) | uniq`.

```
program
├── pipe_sequence
│   ├── pipe_sequence
│   │   ├── pipe_sequence
│   │   │   ├── pipe_sequence
│   │   │   │   └── simple_command
│   │   │   │       ├── cmd_name ── WORD(ls)
│   │   │   │       └── cmd_suffix ── WORD(-la)
│   │   │   ├── '|'
│   │   │   └── simple_command
│   │   │       ├── cmd_name ── WORD(grep)
│   │   │       └── cmd_suffix
│   │   │           ├── cmd_suffix ── WORD(-i)
│   │   │           └── WORD("foo")
│   │   ├── '|'
│   │   └── simple_command
│   │       └── cmd_name ── WORD(sort)
│   ├── '|'
│   └── simple_command
│       └── cmd_name ── WORD(uniq)
└── NEWLINE
```

The parser flattens this into a list of commands (each `|` connects one command's stdout to the next one's stdin):

```
pipeline
├── [0] argv: ls -la
├── [1] argv: grep -i foo
├── [2] argv: sort
└── [3] argv: uniq
```

#### `sort > out.txt -r`

Tokens: `WORD(sort) '>' WORD(out.txt) WORD(-r) NEWLINE`

Redirects can sit anywhere in `cmd_suffix`; the parser pulls them out of argv.

```
program
├── pipe_sequence
│   └── simple_command
│       ├── cmd_name ── WORD(sort)
│       └── cmd_suffix
│           ├── cmd_suffix
│           │   └── io_redirect
│           │       ├── '>'
│           │       └── filename ── WORD(out.txt)
│           └── WORD(-r)
└── NEWLINE
```

Result: `argv: sort -r`, `stdout: out.txt`.

#### `cat < in.txt | wc -l >> count.txt`

Tokens: `WORD(cat) '<' WORD(in.txt) '|' WORD(wc) WORD(-l) DGREAT WORD(count.txt) NEWLINE`

```
program
├── pipe_sequence
│   ├── pipe_sequence
│   │   └── simple_command
│   │       ├── cmd_name ── WORD(cat)
│   │       └── cmd_suffix
│   │           └── io_redirect
│   │               ├── '<'
│   │               └── filename ── WORD(in.txt)
│   ├── '|'
│   └── simple_command
│       ├── cmd_name ── WORD(wc)
│       └── cmd_suffix
│           ├── cmd_suffix ── WORD(-l)
│           └── io_redirect
│               ├── DGREAT
│               └── filename ── WORD(count.txt)
└── NEWLINE
```

Result: `[0] argv: cat, stdin: in.txt` piped to `[1] argv: wc -l, stdout (append): count.txt`.

#### Empty line

Tokens: `NEWLINE`

```
program
└── NEWLINE
```

#### Invalid: `| sort`

Tokens: `'|' WORD(sort) NEWLINE`

No rule starts with `'|'`. `pipe_sequence` must begin with a `simple_command`, which must begin with a `WORD`, so this is a syntax error.

Notes:

- `NEWLINE` may also be `EOF`.
- `TOKEN_STRING` is treated as a `WORD` (POSIX has no separate string token; quotes are part of a word).
- Redirects may appear anywhere among the args, e.g. `sort > out.txt -r`.

Deferred: `;` `&` `&&` `||` `!`, assignments (`FOO=bar cmd`), `IO_NUMBER`/`>&`/`<&`, heredocs,
compound commands (`if`, `for`, `while`, `case`, `{ }`, `( )`), and function definitions.

## Utillity

- Pretty printing AST. Useful for debugging.
