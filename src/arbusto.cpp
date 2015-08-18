/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
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
sequence = ( term | option | repetition ) +
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
	virtual ~grammar_node() {}

	virtual void print() = 0;

	grammar_node_type type;
};

class grammar_node_string : public grammar_node {
public:
	grammar_node_string(const std::string &v) : grammar_node(GNODE_STRING), value(v) {}

	virtual void print() {
		std::cout << "string(" << value << ")";
	}

	std::string value;
};

class grammar_node_optional : public grammar_node {
public:
	grammar_node_optional(grammar_node_ptr &&c) : grammar_node(GNODE_OPTIONAL), child(std::move(c)) {}

	virtual void print() {
		std::cout << "optional(";
		if (child) child->print();
		std::cout << ")";
	}

	grammar_node_ptr child;
};

class grammar_node_repetition : public grammar_node {
public:
	grammar_node_repetition(grammar_node_ptr &&c, bool s) : grammar_node(GNODE_REPETITION), child(std::move(c)), star(s) {}

	virtual void print() {
		std::cout << "repetition(" << (star ? "'*'" : "'+'") << ", ";
		if (child) child->print();
		std::cout << ")";
	}

	grammar_node_ptr child;
	bool star;
};

class grammar_node_sequence : public grammar_node {
public:
	grammar_node_sequence() : grammar_node(GNODE_SEQUENCE) {}

	virtual void print() {
		std::cout << "sequence(";
		for (auto& e : childs) {
			if (e) e->print();
			std::cout << ", ";
		}
		std::cout << ")";
	}

	std::vector<grammar_node_ptr> childs;
};

class grammar_node_rhs : public grammar_node {
public:
	grammar_node_rhs() : grammar_node(GNODE_RHS) {}

	virtual void print() {
		std::cout << "rhs(";
		for (auto& e : choices) {
			if (e) e->print();
			std::cout << ", ";
		}
		std::cout << ")";
	}

	std::vector<grammar_node_ptr> choices;
};

class grammar_node_rule : public grammar_node {
public:
	grammar_node_rule(const std::string& s, grammar_node_ptr r) : grammar_node(GNODE_RULE), rule_name(s), rhs(std::move(r)) {}

	virtual void print() {
		std::cout << "rule(" << rule_name << ", ";
		if (rhs) rhs->print();
		std::cout << ")";
	}

	std::string rule_name;
	grammar_node_ptr rhs;
};

class tokens_iter {
public:
	tokens_iter(size_t begin, size_t end, std::vector<std::string> &tokens)
		: begin_(begin), end_(end), tokens_(tokens)
	{}

	bool eof() {
		return begin_ >= end_;
	}

	const std::string& peek() {
		static std::string empty;
		return eof() ? empty : tokens_.at(begin_);
	}

	const std::string& get() {
		const std::string& ret = peek();
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
	std::map<std::string, grammar_node_ptr> rules;

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

	inline bool is_token_NT(const std::string& s) {
		return s.size() && valid_name_char(s[0]);
	}

	inline bool is_token_T(const std::string& s) {
		return s.size() && s[0] == '\'';
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

grammar_node_ptr grammar::parse_option(tokens_iter& it) {
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

grammar_node_ptr grammar::parse_repetition(tokens_iter& it) {
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

grammar_node_ptr grammar::parse_sequence(tokens_iter& it) {
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

	if (childs.size()) {
		auto node = new grammar_node_sequence();
		node->childs = std::move(childs);
		return grammar_node_ptr(node);
	}

	return grammar_node_ptr();
}

grammar_node_ptr grammar::parse_rhs(tokens_iter& it) {
	/* rhs = sequence ( '|' sequence ) */

	auto p = it.pos();

	auto node = new grammar_node_rhs();
	grammar_node_ptr next;

	next = parse_sequence(it);
	if (!next) {
		it.reset(p);
		return grammar_node_ptr();
	}

	node->choices.push_back(std::move(next));

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

		node->choices.push_back(std::move(next));
		p = it.pos();
	}

	return grammar_node_ptr(node);
}

grammar_node_ptr grammar::parse_rule(tokens_iter& it) {
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

	grammar_node_ptr node = parse_rule(it);

	if (node) {
		node->print();
		std::cout << std::endl;
		grammar_node_rule* rule = static_cast<grammar_node_rule*>(node.get());
		rules[rule->rule_name] = std::move(node);
	} else {
		std::cout << "ERROR for " << p << " " << i << std::endl;
	}
}

int main(int argc, char **argv) {
	grammar G;
	G.parse_grammar_file((argc > 1) ? argv[1] : "Grammar");
	std::cout << "TOKENS COUNT=" << G.tokens.size() << std::endl;
	std::cout << "RULES COUNT=" << G.rules.size() << std::endl;
	return 0;
}
