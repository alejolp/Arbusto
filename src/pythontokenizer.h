/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#ifndef PYTHONTOKENIZER_H_
#define PYTHONTOKENIZER_H_

namespace arbusto {

class python_tokenizer {
public:
	python_tokenizer();
	virtual ~python_tokenizer();

	void tokenize_file(const std::string& file_name);

private:
	std::string detect_encoding_file(const std::string& file_name);

};

} /* namespace arbusto */

#endif /* PYTHONTOKENIZER_H_ */
