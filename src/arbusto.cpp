/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#include <iostream>

#include "grammarparser.h"


int main(int argc, char **argv) {
	arbusto::grammar G;

	G.debug = true;
	G.parse_grammar_file((argc > 1) ? argv[1] : "Grammar");

	if (G.debug) {
		std::cout << "TOKENS COUNT=" << G.tokens.size() << std::endl;
		std::cout << "RULES COUNT=" << G.rules.size() << std::endl;
	}

	return 0;
}
