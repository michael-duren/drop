package main

import (
	"bufio"
	"fmt"
	"os"
)

func main() {
	if len(os.Args) > 2 {
		fmt.Printf("Usage: interpreter [script]")
	} else if len(os.Args) == 2 {
		err := runFile(os.Args[1])
		if err != nil {
			panic(fmt.Sprintf("ah shi here we go again: %v", err.Error()))
		}
	} else {
		err := runPrompt()
		if err != nil {
			panic(fmt.Sprintf("oh yikes dude oof: %v", err.Error()))
		}
	}
}

func runFile(fileName string) error {
	data, err := os.ReadFile(fileName)
	if err != nil {
		return fmt.Errorf("[runFile01] Err opening file: %w\n", err)
	}

	r := NewRunner()
	err = r.Run(data)
	if err != nil {
		return fmt.Errorf("[runFile02] Err running input: %w\n", err)
	}
	return nil
}

func runPrompt() error {
	var input string
	var err error
	r := NewRunner()

	for input != "exit\n" {
		fmt.Print("->")
		inputReader := bufio.NewReader(os.Stdin)
		input, err = inputReader.ReadString('\n')
		if err != nil {
			return fmt.Errorf("[runPrompt01] Err reading input: %w\n", err)
		}

		err = r.Run([]byte(input))
		if err != nil {
			return fmt.Errorf("[runPrompt02] Err running input '%v': %w\n", input, err)
		}
		if r.HadErr {
			r.LastErr.Report()
		}
	}
	return nil
}

