package main

type Token struct {
	Type TokenType
	Lexeme []byte
	Literal any
	Line int
}

