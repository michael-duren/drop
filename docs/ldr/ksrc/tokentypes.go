package main

type TokenType int  
const (
  // Single-character tokens.
  T_OP_LEFT_PAREN TokenType = iota
  T_OP_RIGHT_PAREN
  T_OP_LEFT_BRACE
  T_OP_RIGHT_BRACE
  T_OP_COMMA
  T_OP_DOT
  T_OP_MINUS
  T_OP_PLUS
  T_OP_SEMICOLON
  T_OP_SLASH
  T_OP_STAR

  // One or two character tokens.
  T_OP_BANG
  T_OP_BANG_EQUAL
  T_OP_EQUAL
  T_OP_EQUAL_EQUAL
  T_OP_GREATER
  T_OP_GREATER_EQUAL
  T_OP_LESS
  T_OP_LESS_EQUAL

  // Literals.
  T_LT_IDENTIFIER
  T_LT_STRING
  T_LT_NUMBER
  
  // Keywords.
  T_KW_AND
  T_KW_CLASS
  T_KW_ELSE
  T_KW_FALSE
  T_KW_FUN
  T_KW_FOR
  T_KW_IF
  T_KW_NIL
  T_KW_OR
  T_KW_PRINT
  T_KW_RETURN
  T_KW_SUPER
  T_KW_THIS
  T_KW_TRUE
  T_KW_VAR
  T_KW_WHILE
  T_KW_EOF
)
