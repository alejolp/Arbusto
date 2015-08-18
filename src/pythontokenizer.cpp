/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: MIT
 */

#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <functional>

#include "pythontokenizer.h"

namespace arbusto {

python_tokenizer::python_tokenizer() { }

python_tokenizer::~python_tokenizer() { }

std::string python_tokenizer::detect_encoding_file(const std::string& file_name) {
	std::ifstream ifile;
	int line_counter = 1;
	char c;
	int a;
	std::string tmp;

	tmp.reserve(1024);
	ifile.open(file_name);

	// Empty file
	if (!ifile) return "utf-8";

	// UTF BOM
	a = ifile.get();
	if (a == 0xEF && ifile) {
		a = ifile.get();
		if (a == 0xBB && ifile) {
			a = ifile.get();
			if (a == 0xBF) {
				return "utf-8";
			}
		}
	} else if (a == 0xFE && ifile) {
		a = ifile.get();
		if (a == 0xFF) {
			return "utf-16be";
		}
	} else if (a == 0xFF && ifile) {
		a = ifile.get();
		if (a == 0xFE) {
			return "utf-16le";
		}
	}

	ifile.seekg(0, std::ifstream::beg);

	while (ifile) {
		c = (char) ifile.get();

		if (c == '\n' || c == '\r') {
			char d = ifile ? ifile.get() : 0;
			if (c == '\n' && d != '\r') ifile.unget();
			if (c == '\r' && d != '\n') ifile.unget();
			if (!tmp.empty() && tmp[0] == '#') {
				auto p = tmp.find("coding:");

				if (p == std::string::npos) {
					p = tmp.find("coding=");
				}

				/* #!/usr/bin/env python3
				 * # -*- coding: utf-8 -*-
				 */
				if (p != std::string::npos) {
					size_t a = p + 7, b;
					while (a < tmp.size() && tmp[a] == ' ') ++a;
					b = a;
					while (b < tmp.size() && tmp[b] != ' ') ++b;
					auto coding = tmp.substr(a, b - a);
					std::transform(coding.begin(), coding.end(), coding.begin(), ::tolower);
					return coding;
				}
			}
			tmp.clear();
			line_counter++;
			if (line_counter > 2) break;
		} else {
			tmp += c;
		}
	}

	// Python 3 default
	return "utf-8";
}

void python_tokenizer::tokenize_file(const std::string& file_name) {
	auto file_encoding = detect_encoding_file(file_name);
	std::cout << "file=" << file_name << " encoding=" << file_encoding << std::endl;
}

} /* namespace arbusto */
