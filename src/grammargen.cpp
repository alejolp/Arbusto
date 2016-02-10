/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: BSD
 */

#include <iostream>
#include <string>
#include <set>
#include <stdexcept>

#include "grammargen.h"

namespace arbusto {

bool is_name_terminal(const std::string& name, grammar_parser& G) {
	// Assume the Grammar is fine and every name which is not in G.rules is a terminal, ie, NEWLINE, STRING, etc.
	if (G.rules.find(name) != G.rules.end()) return false;
	return true;
}

void get_FIRST_set(grammar_node* node, std::set<std::string> &S, grammar_parser& G) {
	switch (node->type) {
	case GNODE_STRING: /* for leafs */
		{
			grammar_node_string* nodestr = static_cast<grammar_node_string*>(node);
			auto value = nodestr->value;

			if (is_name_terminal(value, G)) {
				S.insert(value);
			} else if (G.rules.find(value) != G.rules.end()) {
				get_FIRST_set(G.rules[value].get(), S, G);
			} else {
				std::cerr << "!!! Unknown symbol " << value << std::endl;
			}
		}
		break;

	case GNODE_OPTIONAL: /* for [ optional ] */
		{
			grammar_node_optional* nodeopt = static_cast<grammar_node_optional*>(node);

			S.insert("EPS");

			get_FIRST_set(nodeopt->child.get(), S, G);
		}
		break;
		
	case GNODE_REPETITION: /* for X* | X+ */
		{
			grammar_node_repetition* noderep = static_cast<grammar_node_repetition*>(node);

			if (noderep->star) {
				S.insert("EPS");
			}

			get_FIRST_set(noderep->child.get(), S, G);
		}
		break;
		
	case GNODE_SEQUENCE: /* for a sequence A B C */
		{
			grammar_node_sequence* nodeseq = static_cast<grammar_node_sequence*>(node);
			bool found_eps = false;
			size_t idx;

			for (idx = 0; idx != nodeseq->childs.size(); ++idx) {
				std::set<std::string> T;
				get_FIRST_set(nodeseq->childs[idx].get(), T, G);

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
				std::set<std::string> T;
				get_FIRST_set(noderhs->choices[idx].get(), T, G);

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

			get_FIRST_set(noderul->rhs.get(), S, G);
		}
		break;

	default:
		throw std::runtime_error("unknown node type");
	}
}

void generate_parser(grammar_parser& G) {
	for (auto it = G.rules.begin(); it != G.rules.end(); ++it) {
		std::set<std::string> FIRST;
		get_FIRST_set(it->second.get(), FIRST, G);

		std::cout << it->first << ": ";
		for (auto s : FIRST) {
			std::cout << s << ", ";
		}
		std::cout << std::endl;
	}
}

}