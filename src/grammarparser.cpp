/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: BSD
 */

#include <iostream>
#include <fstream>
#include <limits>

#include "grammarparser.h"

namespace arbusto {



/* Tokenizer for the Grammar/Grammar Python file */
void grammar_parser::tokenize_grammar_file(const std::string& file_name) {
	std::ifstream ifile;

	ifile.open(file_name);

	while (ifile) {
		auto c = (char) ifile.get();

		switch (c) {
		/* Eat the WS */
		case ' ':
		case '\t':
			break;

		/* NEWLINE */
		case '\r':
		case '\n':
			break;

		/* Literal string 'hey' */
		case '\'':
			{
				std::string S{c};
				while (ifile) {
					c = ifile.get();
					S += c;
					if (c == '\'') break;
				}
				tokens.emplace_back(S);
			}
			break;

		/* Single character tokens */
		case ':':
		case '|':
		case '(':
		case ')':
		case '[':
		case ']':
		case '+':
		case '*':
			{
				std::string S{c};
				tokens.emplace_back(S);
			}
			break;

		/* Eat the comment until EOL */
		case '#':
			while (ifile) {
				c = ifile.get();
				if (c == '\n') {
					ifile.unget();
					break;
				}
			}
			break;

		/* NAME */
		default:
			if (valid_name_char(c))
			{
				std::string S{c};
				while (ifile) {
					c = ifile.get();
					if (!valid_name_char(c)) {
						ifile.unget();
						break;
					}
					S += c;
				}
				tokens.emplace_back(S);
			}
		}
	}
}

/*
 * Parser for the Grammar/Grammar Python file
 * Implement a Recursive Descent Parser for the Grammar
 *
 * */
void grammar_parser::parse_grammar_file(const std::string& file_name) {
	tokenize_grammar_file(file_name);
	size_t i, p = std::numeric_limits<size_t>::max();

	for (i = 0; i < tokens.size(); ++i) {
		if (tokens[i] == ":") {
			if (p != std::numeric_limits<size_t>::max()) {
				parse_production(p, i - 1);
			}
			p = i - 1;
		}
	}

	if (p != std::numeric_limits<size_t>::max()) {
		parse_production(p, i);
	}
}

grammar_node_ptr grammar_parser::parse_term(tokens_iter& it) {
	/* term = ( NT | T ) [ '+' | '*' ] */

	auto p = it.pos();

	if (it.eof()) {
		it.reset(p);
		return grammar_node_ptr();
	}

	auto next = it.peek();

	if (is_token_NT(next) || is_token_T(next)) {
		it.get();
		grammar_node_ptr node = grammar_node_ptr(new grammar_node_string(next));

		auto next2 = it.peek();
		if (next2 == "*" || next2 == "+") {
			it.get();
			return grammar_node_ptr(new grammar_node_repetition(std::move(node), (next2 == "*")));
		}

		return node;
	}

	it.reset(p);
	return grammar_node_ptr();
}

grammar_node_ptr grammar_parser::parse_option(tokens_iter& it) {
	/* option = '[' rhs ']' */
	auto p = it.pos();

	if (it.peek() != "[") {
		it.reset(p);
		return grammar_node_ptr();
	}

	std::string lbrace = it.get();

	auto rhs = parse_rhs(it);

	if (it.peek() != "]") {
		it.reset(p);
		return grammar_node_ptr();
	}

	std::string rbrace = it.get();

	return grammar_node_ptr(new grammar_node_optional(std::move(rhs)));
}

grammar_node_ptr grammar_parser::parse_repetition(tokens_iter& it) {
	/* repetition = '(' rhs ')' [ '+' | '*' ] */
	auto p = it.pos();

	if (it.peek() != "(") {
		it.reset(p);
		return grammar_node_ptr();
	}

	std::string lpar = it.get();

	auto rhs = parse_rhs(it);

	if (it.peek() != ")") {
		it.reset(p);
		return grammar_node_ptr();
	}

	std::string rpar = it.get();
	std::string next = it.peek();

	if (next == "+" || next == "*") {
		it.get();
		return grammar_node_ptr(new grammar_node_repetition(std::move(rhs), (next == "*")));
	}

	return rhs;
}

grammar_node_ptr grammar_parser::parse_sequence(tokens_iter& it) {
	/* sequence = ( term | option | repetition ) + */

	std::vector<grammar_node_ptr> childs;

	for (;;) {
		grammar_node_ptr next;

		next = parse_term(it);
		if (next) {
			childs.push_back(std::move(next));
			continue;
		}

		next = parse_option(it);
		if (next) {
			childs.push_back(std::move(next));
			continue;
		}

		next = parse_repetition(it);
		if (next) {
			childs.push_back(std::move(next));
			continue;
		}

		break;
	}

	if (childs.size() > 1) {
		auto node = new grammar_node_sequence();
		node->childs = std::move(childs);
		return grammar_node_ptr(node);
	} else if (childs.size() > 0) {
		return std::move(childs[0]);
	}

	return grammar_node_ptr();
}

grammar_node_ptr grammar_parser::parse_rhs(tokens_iter& it) {
	/* rhs = sequence ( '|' sequence ) */

	auto p = it.pos();

	std::vector<grammar_node_ptr> choices;
	grammar_node_ptr next;

	next = parse_sequence(it);
	if (!next) {
		it.reset(p);
		return grammar_node_ptr();
	}

	choices.push_back(std::move(next));

	for (;;) {
		if (it.peek() != "|") {
			break;
		}

		it.get();

		next = parse_sequence(it);
		if (!next) {
			/* FIXME: If this happens: broken text! RULE: A || B */
			it.reset(p);
			break;
		}

		choices.push_back(std::move(next));
		p = it.pos();
	}

	if (choices.size() > 1) {
		auto node = new grammar_node_rhs();
		node->choices = std::move(choices);
		return grammar_node_ptr(node);
	} else {
		return std::move(choices[0]);
	}
}

grammar_node_ptr grammar_parser::parse_rule(tokens_iter& it) {
	/* rule = NT ':' rhs */

	auto p = it.pos();

	auto left = it.peek();

	if (!is_token_NT(left)) {
		it.reset(p);
		return grammar_node_ptr();
	}

	it.get();

	if (it.peek() != ":") {
		it.reset(p);
		return grammar_node_ptr();
	}

	it.get();

	auto rhs = parse_rhs(it);

	if (rhs) {
		return grammar_node_ptr(new grammar_node_rule(left, std::move(rhs)));
	}

	it.reset(p);
	return grammar_node_ptr();
}

void grammar_parser::parse_production(size_t p, size_t i) {
#if 0
	std::cout << p << "; " << i << std::endl;
	while (p < i) {
		std::cout << tokens[p] << ", ";
		++p;
	}
	std::cout << std::endl;
#endif
	tokens_iter it(p, i, tokens);

	grammar_node_ptr node = parse_rule(it);

	if (node) {
		if (debug) {
			node->print();
			std::cout << std::endl;
		}
		grammar_node_rule* rule = static_cast<grammar_node_rule*>(node.get());
		rules[rule->rule_name] = std::move(node);
	} else {
		if (debug) {
			std::cout << "ERROR for " << p << " " << i << std::endl;
		}
	}
}


} /* namespace arbusto */
