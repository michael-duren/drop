package main

import "fmt"

type IntErr struct {
	Line int
	Message string
}

func NewErr(line int, message string) IntErr {
	return IntErr{
		Line: line,
		Message: message,
	}
}

func (e IntErr) Report() {
	fmt.Printf("[line %v] Error: %v\n", e.Line, e.Message)
}
