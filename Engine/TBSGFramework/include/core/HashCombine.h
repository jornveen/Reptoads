#pragma once
#include <functional>

namespace core
{
	// Source: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
	template <typename T>
	static void hash_combine(std::size_t& seed, const T& v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}
}