#pragma once

#include <string>

namespace files {
	std::string load_binary(const char* filepath);
	std::string load_binary(const wchar_t* filepath);
}