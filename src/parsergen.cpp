/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: BSD
 */

#include <iostream>
#include <string>
#include <set>
#include <deque>
#include <stdexcept>

#include "parsergen.h"

namespace arbusto {

struct parser_cache {
	std::map<grammar_node*, size_t > node_code;
	std::map<grammar_node*, std::set<std::string> > FIRST;
};

bool is_name_terminal(const std::string& name, grammar_parser& G) {
	// Assume the Grammar is fine and every name which is not in G.rules is a terminal, ie, NEWLINE, STRING, etc.
	if (G.rules.find(name) != G.rules.end()) return false;
	return true;
}

std::set<std::string> get_FIRST_set(grammar_node* node, grammar_parser& G, parser_cache& C) {
	auto it = C.FIRST.find(node);

	if (it != C.FIRST.end()) {
		return it->second;
	}

	std::set<std::string> S;

	switch (node->type) {
	case GNODE_STRING: /* for leafs */
		{
			grammar_node_string* nodestr = static_cast<grammar_node_string*>(node);
			auto value = nodestr->value;

			if (is_name_terminal(value, G)) {
				S.insert(value);
			} else if (G.rules.find(value) != G.rules.end()) {
				S = get_FIRST_set(G.rules[value].get(), G, C);
			} else {
				std::cerr << "!!! Unknown symbol " << value << std::endl;
			}
		}
		break;

	case GNODE_OPTIONAL: /* for [ optional ] */
		{
			grammar_node_optional* nodeopt = static_cast<grammar_node_optional*>(node);

			S.insert("EPS");

			auto T = get_FIRST_set(nodeopt->child.get(), G, C);
			S.insert(T.begin(), T.end());
		}
		break;
		
	case GNODE_REPETITION: /* for X* | X+ */
		{
			grammar_node_repetition* noderep = static_cast<grammar_node_repetition*>(node);

			if (noderep->star) {
				S.insert("EPS");
			}

			auto T = get_FIRST_set(noderep->child.get(), G, C);
			S.insert(T.begin(), T.end());
		}
		break;
		
	case GNODE_SEQUENCE: /* for a sequence A B C */
		{
			grammar_node_sequence* nodeseq = static_cast<grammar_node_sequence*>(node);
			bool found_eps = false;
			size_t idx;

			for (idx = 0; idx != nodeseq->childs.size(); ++idx) {
				auto T = get_FIRST_set(nodeseq->childs[idx].get(), G, C);

				if (T.find("EPS") == T.end()) {
					S.insert(T.begin(), T.end());
					break;
				} else {
					found_eps = true;
					T.erase("EPS");
					S.insert(T.begin(), T.end());
				}
			}

			if (idx == G.rules.size() && found_eps) {
				S.insert("EPS");
			}
		}
		break;
		
	case GNODE_RHS: /* for alternatives A|B|C */
		{
			grammar_node_rhs* noderhs = static_cast<grammar_node_rhs*>(node);
			size_t idx;

			// None of the alternatives should contain EPS... right? right!

			for (idx = 0; idx != noderhs->choices.size(); ++idx) {
				auto T = get_FIRST_set(noderhs->choices[idx].get(), G, C);

				if (T.find("EPS") != T.end()) {
					std::cerr << "!!! FOUND EPS IN RHS" << std::endl;
					throw std::runtime_error("FOUND EPS IN RHS");
				}

				S.insert(T.begin(), T.end());
			}
		}
		break;
		
	case GNODE_RULE: /* the main rule: NAME ':' RHS */
		{
			grammar_node_rule* noderul = static_cast<grammar_node_rule*>(node);
			S = get_FIRST_set(noderul->rhs.get(), G, C);
		}
		break;

	default:
		throw std::runtime_error("unknown node type");
	}

	C.FIRST[node] = S;
	return S;
}

class node_code_builder : public grammar_node_visitor {
public:
	node_code_builder(std::deque<grammar_node*> &Q_) : Q(Q_) {}

	virtual void visit(grammar_node* node);
	std::deque<grammar_node*> &Q;
};

void node_code_builder::visit(grammar_node* node) {
	Q.push_back(node);
	grammar_node_visitor::visit(node);
}

void build_node_codes(grammar_parser& G, parser_cache& C) {
	std::deque<grammar_node*> Q;
	node_code_builder W(Q);

	for (auto it = G.rules.begin(); it != G.rules.end(); ++it) {
		W.visit(it->second.get());
	}

	while (Q.size() > 0) {
		auto node = Q.front();
		Q.pop_front();
		auto code = C.node_code.size();
		C.node_code[node] = code;
	}
}

void generate_parser(grammar_parser& G) {
	parser_cache C;

	build_node_codes(G, C);

	std::cout << "nodes count: " << C.node_code.size() << std::endl;

	
}

}