/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include <vector>
#include <string>


namespace arbusto {

/* stolen from token.h */

enum token_t {
	TOK_ENDMARKER=0,
	TOK_NAME=1,
	TOK_NUMBER=2,
	TOK_STRING=3,
	TOK_NEWLINE=4,
	TOK_INDENT=5,
	TOK_DEDENT=6,
	TOK_LPAR=7,
	TOK_RPAR=8,
	TOK_LSQB=9,
	TOK_RSQB=10,
	TOK_COLON=11,
	TOK_COMMA=12,
	TOK_SEMI=13,
	TOK_PLUS=14,
	TOK_MINUS=15,
	TOK_STAR=16,
	TOK_SLASH=17,
	TOK_VBAR=18,
	TOK_AMPER=19,
	TOK_LESS=20,
	TOK_GREATER=21,
	TOK_EQUAL=22,
	TOK_DOT=23,
	TOK_PERCENT=24,
	TOK_LBRACE=25,
	TOK_RBRACE=26,
	TOK_EQEQUAL=27,
	TOK_NOTEQUAL=28,
	TOK_LESSEQUAL=29,
	TOK_GREATEREQUAL=30,
	TOK_TILDE=31,
	TOK_CIRCUMFLEX=32,
	TOK_LEFTSHIFT=33,
	TOK_RIGHTSHIFT=34,
	TOK_DOUBLESTAR=35,
	TOK_PLUSEQUAL=36,
	TOK_MINEQUAL=37,
	TOK_STAREQUAL=38,
	TOK_SLASHEQUAL=39,
	TOK_PERCENTEQUAL=40,
	TOK_AMPEREQUAL=41,
	TOK_VBAREQUAL=42,
	TOK_CIRCUMFLEXEQUAL=43,
	TOK_LEFTSHIFTEQUAL=44,
	TOK_RIGHTSHIFTEQUAL=45,
	TOK_DOUBLESTAREQUAL=46,
	TOK_DOUBLESLASH=47,
	TOK_DOUBLESLASHEQUAL=48,
	TOK_AT=49,
	TOK_ATEQUAL=50,
	TOK_RARROW=51,
	TOK_ELLIPSIS=52,
	TOK_OP=53,
	TOK_AWAIT=54,
	TOK_ASYNC=55,
	TOK_ERRORTOKEN=56,
	TOK_N_TOKENS=57
};

struct token {
	token(token_t tok_, size_t pos_, size_t len_, size_t line_num_)
	 : tok(tok_), pos(pos_), len(len_), line_num(line_num_) {}

	token(token_t tok_, size_t pos_, size_t len_, size_t line_num_, const std::string& data_)
	 : tok(tok_), pos(pos_), len(len_), line_num(line_num_), data(data_) {}

	token_t tok;
	size_t pos;
	size_t len;
	size_t line_num;
	std::string data;
};

class tokenizer {
public:
	tokenizer();
	virtual ~tokenizer();

	bool debug{false};

	void tokenize_file(const std::string& file_name, std::vector<token> &toks);
	void tokenize_string(const std::string& file_str, std::vector<token> &toks);

	std::string detect_encoding_file(const std::string& file_name);

	token_t get_next_operator(const std::string& file_str, size_t p, size_t &len);
	bool get_next_string(const std::string& file_str, const size_t p, size_t &len);

	inline bool is_digit_dec(char c) {
		return c >= '0' && c <= '9';
	}

	inline bool is_digit_bin(char c) {
		return c >= '0' && c <= '1';
	}

	inline bool is_digit_hex(char c) {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
	}

	inline bool is_digit_oct(char c) {
		return c >= '0' && c <= '7';
	}

	inline bool is_whitespace(char c) {
		return c == ' ' || c == '\t';
	}

	inline bool is_newline(char c) {
		return c == '\r' || c == '\n';
	}

	inline bool is_ascii_letter(char c) {
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	static std::string token2str(token_t t);
};

} /* namespace arbusto */

#endif /* TOKENIZER_H_ */
