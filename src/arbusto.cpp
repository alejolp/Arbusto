/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#include <iostream>

#include "grammarparser.h"
#include "tokenizer.h"


int main(int argc, char **argv) {
	bool debug = true;

	if (argc >= 3 && std::string(argv[1]) == "parse_grammar") {
		arbusto::grammar_parser G;
		G.debug = debug;
		G.parse_grammar_file(argv[2]);

		if (debug) {
			std::cout << "TOKENS COUNT=" << G.tokens.size() << std::endl;
			std::cout << "RULES COUNT=" << G.rules.size() << std::endl;
		}

		return 0;
	} else if (argc >= 3 && std::string(argv[1]) == "parse_file") {
		arbusto::tokenizer T;
		std::vector<arbusto::token> toks;

		T.debug = debug;
		T.tokenize_file(argv[2], toks);

		for (auto& t : toks) {
			std::cout << arbusto::tokenizer::token2str(t.tok) << " " << t.data << std::endl;
		}

		return 0;
	} else {
		std::cerr << "Usage: " << std::endl;
		std::cerr << " " << argv[0] << " parse_grammar grammar_file" << std::endl;
		std::cerr << " " << argv[0] << " parse_file py_file" << std::endl;
		return 1;
	}

	return 0;
}
