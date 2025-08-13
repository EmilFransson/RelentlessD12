#pragma once

namespace Relentless
{
	constexpr uint64 fnv1a_hash(const char* s, size_t n) noexcept 
	{
		uint64 h = 1469598103934665603ull;
		for (size_t i = 0; i < n; ++i) 
		{ 
			h ^= uint64(s[i]); h *= 1099511628211ull; 
		}
		
		return h;
	}

	constexpr uint64 operator"" _h(const char* s, size_t n) noexcept 
	{
		return fnv1a_hash(s, n);
	}

	namespace StringUtils
	{
		NO_DISCARD String ConvertFromWide(const std::wstring& input) noexcept;
		NO_DISCARD std::wstring ConvertToWide(const String& input) noexcept;
		NO_DISCARD std::vector<String> Split(const String& input, char delimiter) noexcept;
		NO_DISCARD String ToLower(const String& input) noexcept;
	}
}