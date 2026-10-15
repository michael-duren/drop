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


