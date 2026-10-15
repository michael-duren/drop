package main

type Scanner struct {
	data []byte
}

func NewScanner(data []byte) Scanner {
	return Scanner{
		data: data,
	}
}

// Just splits on space delimiter for now 
// Would need to account for logical grouping of chars by type
func (s *Scanner) ScanTokens() []Token {
	t := Token{}
	tl := make([]Token, 0)

	for _, b := range s.data {
		if b == ' ' || b == '\n' {

			if len(t.Lexeme) > 0 {
				tl = append(tl, t)
				t = Token{}
			}

		} else {
			t.Lexeme = append(t.Lexeme, b)
		}
	}
	return tl
}
