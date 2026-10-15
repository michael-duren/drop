package main

import "fmt"

type Runner struct {
	HadErr bool
	LastErr IntErr
}

func NewRunner() Runner {
	return Runner{}
}

func (r *Runner) Run(data []byte) error {
	s := NewScanner(data)
	tl := s.ScanTokens()

	for _, t := range tl {
		fmt.Printf("%s", t.Lexeme)
	}
	fmt.Println("")

	return nil
}
