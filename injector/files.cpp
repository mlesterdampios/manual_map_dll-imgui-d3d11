#pragma once

#include "files.h"

#include <fstream>

namespace files {
	std::string load_binary(const char* filepath) {
		std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
		std::string result;

		if (!ifs)
			return result;

		auto end = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		auto size = std::size_t(end - ifs.tellg());
		if (size == 0) // avoid undefined behavior 
			return {};

		result.resize(size);
		if (!ifs.read((char*)result.data(), result.size()))
			return result;

		return result;
	}

	std::string load_binary(const wchar_t* filepath) {
		std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
		std::string result;

		if (!ifs)
			return result;

		auto end = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		auto size = std::size_t(end - ifs.tellg());
		if (size == 0) // avoid undefined behavior 
			return {};

		result.resize(size);
		if (!ifs.read((char*)result.data(), result.size()))
			return result;

		return result;
	}
};