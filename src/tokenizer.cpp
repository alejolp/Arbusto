/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: BSD
 */

#include "tokenizer.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <functional>
#include <vector>
#include <stdexcept>


namespace arbusto {

tokenizer::tokenizer() {
}

tokenizer::~tokenizer() {
}

std::string tokenizer::detect_encoding_file(const std::string& file_name) {
	std::ifstream ifile;
	int line_counter = 1;
	char c;
	int a;
	std::string tmp;

	tmp.reserve(1024);
	ifile.open(file_name);

	// Empty file
	if (!ifile)
		return "utf-8";

	// UTF BOM
	a = ifile.get();
	if (a == 0xEF && ifile) {
		a = ifile.get();
		if (a == 0xBB && ifile) {
			a = ifile.get();
			if (a == 0xBF) {
				return "utf-8";
			}
		}
	} else if (a == 0xFE && ifile) {
		a = ifile.get();
		if (a == 0xFF) {
			return "utf-16be";
		}
	} else if (a == 0xFF && ifile) {
		a = ifile.get();
		if (a == 0xFE) {
			return "utf-16le";
		}
	}

	ifile.seekg(0, std::ifstream::beg);

	while (ifile) {
		c = (char) ifile.get();

		if (c == '\n' || c == '\r') {
			char d = ifile ? ifile.get() : 0;
			if (c == '\n' && d != '\r')
				ifile.unget();
			if (c == '\r' && d != '\n')
				ifile.unget();
			if (!tmp.empty() && tmp[0] == '#') {
				auto p = tmp.find("coding:");

				if (p == std::string::npos) {
					p = tmp.find("coding=");
				}

				/* #!/usr/bin/env python3
				 * # -*- coding: utf-8 -*-
				 */
				if (p != std::string::npos) {
					size_t a = p + 7, b;
					while (a < tmp.size() && tmp[a] == ' ')
						++a;
					b = a;
					while (b < tmp.size() && tmp[b] != ' ')
						++b;
					auto coding = tmp.substr(a, b - a);
					std::transform(coding.begin(), coding.end(), coding.begin(), ::tolower);
					return coding;
				}
			}
			tmp.clear();
			line_counter++;
			if (line_counter > 2)
				break;
		} else {
			tmp += c;
		}
	}

	// Python 3 default
	return "utf-8";
}

std::string tokenizer::token2str(token_t t) {
	/*
def buildtok2str(S):
	return '\n'.join([('case ' + x.strip().split('=')[0] + ': return "' + x.strip().split('=')[0] + '";') for x in S.split('\n') if x.strip()])
	 */
	switch (t) {
	case TOK_ENDMARKER:
		return "TOK_ENDMARKER";
	case TOK_NAME:
		return "TOK_NAME";
	case TOK_NUMBER:
		return "TOK_NUMBER";
	case TOK_STRING:
		return "TOK_STRING";
	case TOK_NEWLINE:
		return "TOK_NEWLINE";
	case TOK_INDENT:
		return "TOK_INDENT";
	case TOK_DEDENT:
		return "TOK_DEDENT";
	case TOK_LPAR:
		return "TOK_LPAR";
	case TOK_RPAR:
		return "TOK_RPAR";
	case TOK_LSQB:
		return "TOK_LSQB";
	case TOK_RSQB:
		return "TOK_RSQB";
	case TOK_COLON:
		return "TOK_COLON";
	case TOK_COMMA:
		return "TOK_COMMA";
	case TOK_SEMI:
		return "TOK_SEMI";
	case TOK_PLUS:
		return "TOK_PLUS";
	case TOK_MINUS:
		return "TOK_MINUS";
	case TOK_STAR:
		return "TOK_STAR";
	case TOK_SLASH:
		return "TOK_SLASH";
	case TOK_VBAR:
		return "TOK_VBAR";
	case TOK_AMPER:
		return "TOK_AMPER";
	case TOK_LESS:
		return "TOK_LESS";
	case TOK_GREATER:
		return "TOK_GREATER";
	case TOK_EQUAL:
		return "TOK_EQUAL";
	case TOK_DOT:
		return "TOK_DOT";
	case TOK_PERCENT:
		return "TOK_PERCENT";
	case TOK_LBRACE:
		return "TOK_LBRACE";
	case TOK_RBRACE:
		return "TOK_RBRACE";
	case TOK_EQEQUAL:
		return "TOK_EQEQUAL";
	case TOK_NOTEQUAL:
		return "TOK_NOTEQUAL";
	case TOK_LESSEQUAL:
		return "TOK_LESSEQUAL";
	case TOK_GREATEREQUAL:
		return "TOK_GREATEREQUAL";
	case TOK_TILDE:
		return "TOK_TILDE";
	case TOK_CIRCUMFLEX:
		return "TOK_CIRCUMFLEX";
	case TOK_LEFTSHIFT:
		return "TOK_LEFTSHIFT";
	case TOK_RIGHTSHIFT:
		return "TOK_RIGHTSHIFT";
	case TOK_DOUBLESTAR:
		return "TOK_DOUBLESTAR";
	case TOK_PLUSEQUAL:
		return "TOK_PLUSEQUAL";
	case TOK_MINEQUAL:
		return "TOK_MINEQUAL";
	case TOK_STAREQUAL:
		return "TOK_STAREQUAL";
	case TOK_SLASHEQUAL:
		return "TOK_SLASHEQUAL";
	case TOK_PERCENTEQUAL:
		return "TOK_PERCENTEQUAL";
	case TOK_AMPEREQUAL:
		return "TOK_AMPEREQUAL";
	case TOK_VBAREQUAL:
		return "TOK_VBAREQUAL";
	case TOK_CIRCUMFLEXEQUAL:
		return "TOK_CIRCUMFLEXEQUAL";
	case TOK_LEFTSHIFTEQUAL:
		return "TOK_LEFTSHIFTEQUAL";
	case TOK_RIGHTSHIFTEQUAL:
		return "TOK_RIGHTSHIFTEQUAL";
	case TOK_DOUBLESTAREQUAL:
		return "TOK_DOUBLESTAREQUAL";
	case TOK_DOUBLESLASH:
		return "TOK_DOUBLESLASH";
	case TOK_DOUBLESLASHEQUAL:
		return "TOK_DOUBLESLASHEQUAL";
	case TOK_AT:
		return "TOK_AT";
	case TOK_ATEQUAL:
		return "TOK_ATEQUAL";
	case TOK_RARROW:
		return "TOK_RARROW";
	case TOK_ELLIPSIS:
		return "TOK_ELLIPSIS";
	case TOK_OP:
		return "TOK_OP";
	case TOK_AWAIT:
		return "TOK_AWAIT";
	case TOK_ASYNC:
		return "TOK_ASYNC";
	case TOK_ERRORTOKEN:
		return "TOK_ERRORTOKEN";
	case TOK_N_TOKENS:
		return "TOK_N_TOKENS";
	default:
		return "<unknown>";
	}
}

void tokenizer::tokenize_file(const std::string& file_name, std::vector<token> &toks) {
	auto file_encoding = detect_encoding_file(file_name);
	std::string file_str;

	if (debug) {
		std::cout << "file=" << file_name << " encoding=" << file_encoding << std::endl;
	}

	/* FIXME USE ENCODING */

	{
		std::ifstream ifile;

		ifile.open(file_name);

		ifile.seekg(0, std::ios::end);
		file_str.reserve(ifile.tellg());
		ifile.seekg(0, std::ios::beg);

		file_str.assign((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
	}

	tokenize_string(file_str, toks);
}

void tokenizer::tokenize_string(const std::string& file_str, std::vector<token> &toks) {
	size_t p = 0;
	size_t line_num = 1;
	int nest_level = 0;
	bool line_new = true;
	std::vector<size_t> indent_stack;

	indent_stack.push_back(0);

	while (p < file_str.size()) {
		if (is_whitespace(file_str[p]))
		{
			size_t i = p;
			while (p < file_str.size() && is_whitespace(file_str[p])) {
				++p;
			}
			if (line_new) {
				line_new = false;
				/* INDENT */
				/* is line blank? */
				if (p < file_str.size() && file_str[p] != '#' && !is_newline(file_str[p])) {
					if (nest_level == 0) {
						size_t dist = p - i;

						if (dist > indent_stack.back()) {
							toks.emplace_back(TOK_INDENT, i, dist, line_num);
						} else if (dist > indent_stack.back()) {
							while (dist > indent_stack.back()) {
								toks.emplace_back(TOK_DEDENT, i, dist, line_num);
								indent_stack.pop_back();
							}
						}
					}
				}
			}
		}
		else if (is_newline(file_str[p]))
		{
			if ((toks.size() && toks.back().tok != TOK_NEWLINE) && nest_level == 0 and !line_new) {
				toks.emplace_back(TOK_NEWLINE, p, 1, line_num, "\n");
			}
			++p;
			++line_num;
			if (nest_level == 0) {
				line_new = true;
			}
		}
		else if (line_new)
		{
			line_new = false;
			/* if we reach here means the next token is not whitespace, we have zero indent, and following token is a stmt */
			while (0 < indent_stack.back()) {
				toks.emplace_back(TOK_DEDENT, p, 0, line_num);
				indent_stack.pop_back();
			}
		}
		else if (file_str[p] == '#')
		{
			/* comment */
			while (p < file_str.size() && !is_newline(file_str[p])) {
				++p;
			}
		}
		else if ((p + 1) < file_str.size() && file_str[p] == '\\' && is_newline(file_str[p + 1]))
		{
			/* next line follows this \ */
			++p;
			++line_num;
		}
		else if (is_digit_dec(file_str[p]) || ((p + 1) < file_str.size() && file_str[p] == '.' && is_digit_dec(file_str[p + 1])))
		{
			/* number */
			auto c1 = file_str[p];
			auto c2 = ((p + 1) < file_str.size()) ? file_str[p + 1] : ' ';
			auto i = p;

			if (c1 == '0' && (c2 == 'x' || c2 == 'X')) {
				/* hex */
				p += 2;
				while (p < file_str.size() && is_digit_hex(file_str[p])) {
					++p;
				}
				if (p - i >= 3) {
					toks.emplace_back(TOK_NUMBER, i, p - i, line_num, file_str.substr(i, p - i));
				} else {
					throw std::runtime_error("tokenizer error: digits missing at ptr=" + std::to_string(p));
				}
			} else if (c1 == '0' && (c2 == 'b' || c2 == 'B')) {
				/* bin */
				p += 2;
				while (p < file_str.size() && is_digit_bin(file_str[p])) {
					++p;
				}
				if (p - i >= 3) {
					toks.emplace_back(TOK_NUMBER, i, p - i, line_num, file_str.substr(i, p - i));
				} else {
					throw std::runtime_error("tokenizer error: digits missing at ptr=" + std::to_string(p));
				}
			} else if (c1 == '0' && (c2 == 'o' || c2 == 'O')) {
				/* oct */
				p += 2;
				while (p < file_str.size() && is_digit_oct(file_str[p])) {
					++p;
				}
				if (p - i >= 3) {
					toks.emplace_back(TOK_NUMBER, i, p - i, line_num, file_str.substr(i, p - i));
				} else {
					throw std::runtime_error("tokenizer error: digits missing at ptr=" + std::to_string(p));
				}
			} else {
				/* dec */
				while (p < file_str.size() && is_digit_dec(file_str[p])) {
					++p;
				}

				if (p < file_str.size() && file_str[p] == '.') {
					/* floats 3.14 */
					++p;
					while (p < file_str.size() && is_digit_dec(file_str[p])) {
						++p;
					}
				}

				if (p < file_str.size() && (file_str[p] == 'e' || file_str[p] == 'E')) {
					++p;
					if (p < file_str.size() && file_str[p] == '-') {
						++p;
					}
					auto k = p;
					while (p < file_str.size() && is_digit_dec(file_str[p])) {
						++p;
					}
					if (p - k < 1) {
						throw std::runtime_error("tokenizer error: exp part missing at ptr=" + std::to_string(p));
					}
				}

				toks.emplace_back(TOK_NUMBER, i, p - i, line_num, file_str.substr(i, p - i));
			}
		}
		else
		{
			/* operators */
			{
				size_t tlen = 0;
				auto t = get_next_operator(file_str, p, tlen);

				if (t != TOK_N_TOKENS) {
					toks.emplace_back(t, p, tlen, line_num, file_str.substr(p, tlen));
					p += tlen;

					switch (t) {
					case TOK_LPAR:
					case TOK_LBRACE:
					case TOK_LSQB:
						nest_level++;
						break;
					case TOK_RPAR:
					case TOK_RBRACE:
					case TOK_RSQB:
						nest_level--;
						break;

					default:
						break;
					}

					if (nest_level < 0) {
						throw std::runtime_error("tokenizer error: nest level negative at ptr=" + std::to_string(p));
					}

					continue;
				}
			}

			/* string literals */
			{
				size_t tlen = 0;
				if (get_next_string(file_str, p, tlen)) {
					toks.emplace_back(TOK_STRING, p, tlen, line_num, file_str.substr(p, tlen));
					p += tlen;
					continue;
				}
			}

			/* names */
			if (is_ascii_letter(file_str[p])) {
				size_t k = p;
				while (p < file_str.size() && (is_ascii_letter(file_str[p]) || is_digit_bin(file_str[p]) || file_str[p] == '_')) {
					++p;
				}
				toks.emplace_back(TOK_NAME, k, p - k, line_num, file_str.substr(k, p - k));
				continue;
			}

			throw std::runtime_error("tokenizer error at ptr=" + std::to_string(p));
		}
	}

	toks.emplace_back(TOK_ENDMARKER, p, 0, line_num);
}

bool tokenizer::get_next_string(const std::string& file_str, const size_t p, size_t &len) {
	auto c1 = std::tolower(file_str[p]);
	auto c2 = (p + 1 < file_str.size()) ? std::tolower(file_str[p + 1]) : ' ';

	len = 0;

	if (c1 == 'u') {
		len++;
	} else if (c1 == 'r') {
		len++;
		if (c2 == 'b') {
			len++;
		}
	} else if (c1 == 'b') {
		len++;
		if (c2 == 'r') {
			len++;
		}
	}

	auto quote_char = (p + len < file_str.size()) ? file_str[p + len] : ' ';

	if (quote_char == '"' || quote_char == '\'') {
		if (p + len + 3 < file_str.size()) {
			bool long_quote = ((quote_char == file_str[p + len + 1]) && (quote_char == file_str[p + len + 2]));
			bool found = false;
			size_t k;

			if (long_quote) {
				k = p + len + 3;
				while (k + 2 < file_str.size()) {
					if (file_str[k] == '\\' && (file_str[k + 1] == '"' || file_str[k + 1] == '\'')) {
						k += 2;
					} else if (quote_char == file_str[k] && quote_char == file_str[k + 1]
							&& quote_char == file_str[k + 2]) {
						found = true;
						k += 3;
						break;
					} else {
						++k;
					}
				}
			} else {
				k = p + len + 1;
				while (k < file_str.size()) {
					if (file_str[k] == '\\' && (file_str[k + 1] == '"' || file_str[k + 1] == '\'')) {
						k += 2;
					} else if (is_newline(file_str[k])) {
						throw std::runtime_error("tokenizer error: missing closing quotes at ptr=" + std::to_string(k));
					} else if (file_str[k] == quote_char) {
						found = true;
						k += 1;
						break;
					} else {
						++k;
					}
				}
			}

			if (!found) {
				throw std::runtime_error("tokenizer error: missing closing quotes at ptr=" + std::to_string(k));
			}

			len = k - p;
			return true;
		}
	}

	return false;
}

token_t tokenizer::get_next_operator(const std::string& file_str, size_t p, size_t &len) {
	auto c1 = file_str[p];
	auto c2 = (p + 1 < file_str.size()) ? file_str[p + 1] : ' ';
	auto c3 = (p + 2 < file_str.size()) ? file_str[p + 2] : ' ';

	switch (c1) {
	case '(':
		len = 1;
		return TOK_LPAR;
	case ')':
		len = 1;
		return TOK_RPAR;
	case '[':
		len = 1;
		return TOK_LSQB;
	case ']':
		len = 1;
		return TOK_RSQB;
	case ':':
		len = 1;
		return TOK_COLON;
	case ',':
		len = 1;
		return TOK_COMMA;
	case ';':
		len = 1;
		return TOK_SEMI;
	case '.':
		switch (c2) {
		case '.':
			switch (c3) {
			case '.':
				len = 3;
				return TOK_ELLIPSIS;
			}
			break;
		}
		len = 1;
		return TOK_DOT;
	case '{':
		len = 1;
		return TOK_LBRACE;
	case '}':
		len = 1;
		return TOK_RBRACE;
	case '~':
		len = 1;
		return TOK_TILDE;
	case '@':
		len = 1;
		return TOK_AT;
	case '<':
		switch (c2) {
		case '>':
			len = 2;
			return TOK_NOTEQUAL;
		case '=':
			len = 2;
			return TOK_LESSEQUAL;
		case '<':
			switch (c3) {
			case '=':
				len = 3;
				return TOK_LEFTSHIFTEQUAL;
			default:
				len = 2;
				return TOK_LEFTSHIFT;
			}
			break;
		default:
			len = 1;
			return TOK_LESS;
		}
		break;
	case '>':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_GREATEREQUAL;
		case '>':
			switch (c3) {
			case '=':
				len = 3;
				return TOK_RIGHTSHIFTEQUAL;
			default:
				len = 2;
				return TOK_RIGHTSHIFT;
			}
			break;
		default:
			len = 1;
			return TOK_GREATER;
		}
		break;
	case '=':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_EQEQUAL;
		default:
			len = 1;
			return TOK_EQUAL;
		}
		break;
	case '!':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_NOTEQUAL;
		default:
			return TOK_N_TOKENS;
		}
		break;
	case '+':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_PLUSEQUAL;
		default:
			len = 1;
			return TOK_PLUS;
		}
		break;
	case '-':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_MINEQUAL;
		case '>':
			len = 2;
			return TOK_RARROW;
		default:
			len = 1;
			return TOK_MINUS;
		}
		break;
	case '*':
		switch (c2) {
		case '*':
			switch (c3) {
			case '=':
				len = 3;
				return TOK_DOUBLESTAREQUAL;
			default:
				len = 2;
				return TOK_DOUBLESTAR;
			}
			break;
		case '=':
			len = 2;
			return TOK_STAREQUAL;
		default:
			len = 1;
			return TOK_STAR;
		}
		break;
	case '/':
		switch (c2) {
		case '/':
			switch (c3) {
			case '=':
				len = 3;
				return TOK_DOUBLESLASHEQUAL;
			default:
				len = 2;
				return TOK_DOUBLESLASH;
			}
			break;
		case '=':
			len = 2;
			return TOK_SLASHEQUAL;
		default:
			len = 1;
			return TOK_SLASH;
		}
		break;
	case '|':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_VBAREQUAL;
		default:
			len = 1;
			return TOK_VBAR;
		}
		break;
	case '%':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_PERCENTEQUAL;
		default:
			len = 1;
			return TOK_PERCENT;
		}
		break;
	case '&':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_AMPEREQUAL;
		default:
			len = 1;
			return TOK_AMPER;
		}
		break;
	case '^':
		switch (c2) {
		case '=':
			len = 2;
			return TOK_CIRCUMFLEXEQUAL;
		default:
			len = 1;
			return TOK_CIRCUMFLEX;
		}
		break;
	}

	return TOK_N_TOKENS;
}


} /* namespace arbusto */
