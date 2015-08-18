/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#ifndef GRAMMARPARSER_H_
#define GRAMMARPARSER_H_

#include <vector>
#include <string>
#include <map>
#include <memory>


namespace arbusto {


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
		int i=0;
		for (auto& e : choices) {
			if (e) {
				if (i++) std::cout << " | ";
				e->print();
			}
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

	bool debug{false};
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


} /* namespace arbusto */

#endif /* GRAMMARPARSER_H_ */
