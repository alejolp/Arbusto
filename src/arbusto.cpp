/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <limits>
#include <memory>

class grammar_node;
class grammar;

/*

EBNF for Grammar/Grammar
========================

term = ( NT | T ) [ '+' | '*' ]
option = '[' rhs ']'
repetition = '(' rhs ')' [ '+' | '*' ]
sequence = ( term | option | repetition ) *
rhs = sequence ( '|' sequence ) *
rule = NT ':' rhs

 */

/* IMPORTANT: The EBNF does not exactly matches the generated tree */
enum grammar_node_type {
	GNODE_STRING, /* for leafs */
	GNODE_OPTIONAL, /* for [ optional ] */
	GNODE_REPETITION, /* for X* | X+ */
	GNODE_SEQUENCE, /* for a sequence A B C */
	GNODE_RHS, /* for alternatives A|B|C */
	GNODE_RULE /* the main rule: NAME ':' RHS */
};

typedef std::unique_ptr<grammar_node> grammar_node_ptr;

class grammar_node {
public:
	explicit grammar_node(grammar_node_type t) : type(t) {}

	grammar_node_type type;
};

class grammar_node_string : public grammar_node {
public:
	grammar_node_string(const std::string &v) : grammar_node(GNODE_STRING), value(v) {}

	std::string value;
};

class grammar_node_optional : public grammar_node {
public:
	grammar_node_optional(grammar_node_ptr c) : grammar_node(GNODE_OPTIONAL), child(c) {}

	grammar_node_ptr child;
};

class grammar_node_repetition : public grammar_node {
public:
	grammar_node_repetition(grammar_node_ptr c, bool s) : grammar_node(GNODE_REPETITION), child(c), star(s) {}

	grammar_node_ptr child;
	bool star;
};

class grammar_node_sequence : public grammar_node {
public:
	grammar_node_sequence() : grammar_node(GNODE_SEQUENCE) {}

	std::vector<grammar_node_ptr> childs;
};

class grammar_node_rhs : public grammar_node {
public:
	grammar_node_rhs() : grammar_node(GNODE_RHS) {}

	std::vector<grammar_node_ptr> choices;
};

class grammar_node_rule : public grammar_node {
public:
	grammar_node_rule() : grammar_node(GNODE_RULE) {}

	std::string rule_name;
	grammar_node_ptr rhs;
};

class tokens_iter {
public:
	tokens_iter(size_t begin, size_t end, std::vector<std::string> &tokens)
		: begin_(begin), end_(end), tokens_(tokens)
	{}

	bool eof() {
		return end_ >= begin_;
	}

	const std::string& peek() {
		return eof() ? "" : tokens_.at(begin_);
	}

	const std::string& get() {
		auto ret = peek();
		begin_++;
		return ret;
	}

	size_t pos() {
		return begin_;
	}

	void reset(size_t p) {
		begin_ = p;
	}

private:
	size_t begin_;
	size_t end_;
	std::vector<std::string> &tokens_;
};

class grammar {
public:
	void parse_grammar_file(const std::string& file_name);

	std::vector<std::string> tokens;

private:
	void tokenize_grammar_file(const std::string& file_name);

	void parse_production(size_t p, size_t i);

	grammar_node_ptr parse_term(tokens_iter& it);
	grammar_node_ptr parse_option(tokens_iter& it);
	grammar_node_ptr parse_repetition(tokens_iter& it);
	grammar_node_ptr parse_sequence(tokens_iter& it);
	grammar_node_ptr parse_rhs(tokens_iter& it);
	grammar_node_ptr parse_rule(tokens_iter& it);

	inline bool valid_name_char(int c) {
		return std::isalnum(c) || c == '_';
	}
};

/* Tokenizer for the Grammar/Grammar Python file */
void grammar::tokenize_grammar_file(const std::string& file_name) {
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
void grammar::parse_grammar_file(const std::string& file_name) {
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

grammar_node_ptr grammar::parse_term(tokens_iter& it) {

}

grammar_node_ptr grammar::parse_option(tokens_iter& it) {
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

	grammar_node_ptr ret = grammar_node::option();
	ret->childs.push_back(lbrace);
	ret->childs.push_back(rhs);
	ret->childs.push_back(rbrace);

	return ret;
}

grammar_node_ptr grammar::parse_repetition(tokens_iter& it) {

}

grammar_node_ptr grammar::parse_sequence(tokens_iter& it) {

}

grammar_node_ptr grammar::parse_rhs(tokens_iter& it) {

}

grammar_node_ptr grammar::parse_rule(tokens_iter& it) {

}

void grammar::parse_production(size_t p, size_t i) {
#if 0
	std::cout << p << "; " << i << std::endl;
	while (p < i) {
		std::cout << tokens[p] << ", ";
		++p;
	}
	std::cout << std::endl;
#endif
	tokens_iter it(p, i, tokens);
}

int main(int argc, char **argv) {
	grammar G;
	G.parse_grammar_file(argv[1]);
	std::cout << G.tokens.size() << std::endl;
	return 0;
}
