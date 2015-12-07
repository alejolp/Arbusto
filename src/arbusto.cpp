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
	arbusto::grammar G;

	G.debug = debug;
	G.parse_grammar_file((argc > 1) ? argv[1] : "Grammar");

	if (debug) {
		std::cout << "TOKENS COUNT=" << G.tokens.size() << std::endl;
		std::cout << "RULES COUNT=" << G.rules.size() << std::endl;
	}

	arbusto::tokenizer T;
	std::vector<arbusto::token> toks;

	T.debug = debug;
	T.tokenize_file("../../pppp/test.py", toks);

	for (auto& t : toks) {
		std::cout << arbusto::tokenizer::token2str(t.tok) << " " << t.data << std::endl;
	}

	return 0;
}
