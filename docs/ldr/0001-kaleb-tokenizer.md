# Crafting Interpreters Notes
_A Notes By Kaleb Olson_

Reference material -- [Crafting Interpreters](https://craftinginterpreters.com/scanning.html)

## Scanning

Effectively, read a set of chars and break them down into _tokens_. 

> Scanning == lexing (ie. lexical analysis).

The smallest possible sequence that represents something meaningful is also called a __lexeme__.
The __lexeme__ is the raw string value, where a __token__ is the lexeme combined with other metadata. This metadata includes things like: 
- Token Type
    - keywords, syntax, etc
- Literal Value
    - numbers, strings
- Location Information
    - line number, potentially including column/length as well

You can represent all this in a Token struct

> Simple languages can be classified as "regular" languages, same as the regular in regex. 
> You can use regex for lexical matching, but the lesson uses a scanner instead for learning purposes. 

## Representing code

Languages should have a grammar defined that can be used to build the syntax tree.

basic grammar for lox example:
```
expression     → literal
               | unary
               | binary
               | grouping ;

literal        → NUMBER | STRING | "true" | "false" | "nil" ;
grouping       → "(" expression ")" ;
unary          → ( "-" | "!" ) expression ;
binary         → expression operator expression ;
operator       → "==" | "!=" | "<" | "<=" | ">" | ">="
               | "+"  | "-"  | "*" | "/" ;
```

### Expression problem

OOP vs FP focus on two different solutions -- 
- OOP: easier to add data structures (rows)
- FP: easier to add functions (columns)

FP works a little better for our use case, as we have several types of tokens (structs) but it is fixed set.
Use pattern matching to handle appropriate behavior based on provided data structure


## Parsing Expressions

Where we can use our grammar to generate any number of strings, the parser must do the opposite and identify the rules used to generate a string.

An order of __precedence__ must be established to determine order of evaluation of operators in an expression.
__associativity__ is similar to precedence but for the same operator in the same expression. Either left associative or right associative.

**Recursive Descent**: a simple top-down parsing technique used by gcc, v8, and roslyn

Modified lox grammar with associativity and precedence, meaning ambiguity resolved 

```
expression     → equality ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" ;
```

Create methods for each rule in the heirarchy:
```
expression() {
  return equality()
}

equality() {
  expr = comparison()

  while (current at bang_equal or equal_equal) {
    op = prev()
    right = comparison()
    expr = new binary_expression(expr, op, right)
  }
  return expr
}

comparison() {
  expr = term()

  while (current at greater, greater_eq, less, less_eq) {
    op = prev()
    right = term()
    expr = binary_expression(expr, op, right)
  }
}

// implement term and onward down the tree
// once you reach unary its a bit different 
unary() {
  if (current at bang or minus) {
    op = prev()
    right = unary()
    return unary_expression(op, right)
  }
}

// then do primary/literals finally, which all just return different type of literal (true/false/nil/number/string) or a grouping (())
```

when encountering an error in the parser, try to handle it appropriately rather than crashing. Errors are you compiler's frontend
report errors, do it quickly, report as many distinct errors as you can while, avoiding cascading errors

