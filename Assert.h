#pragma once

#include <iostream>

inline bool assert_line_trace(bool cond, const char* file, const char* function, int line, std::string msg)
{
	if (!(cond)) std::cerr << "[ASSERT]: " << msg << " " << function << "() @ " << file << ":" << line << std::endl;

	return !(cond);
}

#define ASSERT_TRUE(cond, msg) assert_line_trace(cond, __FILE__, __func__, __LINE__, (msg))