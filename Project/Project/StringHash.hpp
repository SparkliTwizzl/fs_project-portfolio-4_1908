// i did not come up with this, i got it from CompilerExplorer via StackOverflow:
// https://gcc.godbolt.org/z/pfJXRm
// https://stackoverflow.com/a/46711735

#pragma once

#include <string>


constexpr inline long long int hash(const char* str, int hashValue);
inline long long int hash(const std::string& str);
constexpr inline long long int operator "" _(const char* str, size_t);



constexpr inline long long int hash(const char* str, int hashValue = 0)
{
	return (!str[hashValue] ? 5381 : (hash(str, hashValue + 1) * 33) ^ str[hashValue]);
}

inline long long int hash(std::string const& str)
{
	return hash(str.c_str());
}

constexpr inline long long int operator "" _(const char* str, size_t)
{
	return hash(str);
}
