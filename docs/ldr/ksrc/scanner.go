package main

type Scanner struct {
	data []byte
	Tokens []Token
}

func NewScanner(data []byte) Scanner {
	return Scanner{
		data: data,
		Tokens: make([]Token, 0),
	}
}

// Just splits on space delimiter for now
// Would need to account for logical grouping of chars by type
func (s *Scanner) ScanTokens() {
	for i := 0; i < len(s.data); {
		s.ScanToken(&i)
	}

	s.Tokens = append(s.Tokens, Token{Type: T_KW_EOF})
}

func (s *Scanner) ScanToken(idx *int) {
	datalen := len(s.data)
	for *idx < datalen && s.data[*idx] == ' ' {
		*idx++
	}

	switch s.data[*idx] {
      case '(': s.addToken(T_OP_LEFT_PAREN)
      case ')': s.addToken(T_OP_RIGHT_PAREN)
      case '{': s.addToken(T_OP_LEFT_BRACE)
      case '}': s.addToken(T_OP_RIGHT_BRACE)
      case ',': s.addToken(T_OP_COMMA)
      case '.': s.addToken(T_OP_DOT)
      case '-': s.addToken(T_OP_MINUS)
      case '+': s.addToken(T_OP_PLUS)
      case ';': s.addToken(T_OP_SEMICOLON)
      case '*': s.addToken(T_OP_STAR) 
	}
}

func (s *Scanner) addToken(tt TokenType) {
	t := Token{
		Type: tt,

	}
}
