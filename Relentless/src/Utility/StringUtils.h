#pragma once
#include "Core/DLLExport.h"

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

	constexpr uint64 operator""_h(const char* s, size_t n) noexcept 
	{
		return fnv1a_hash(s, n);
	}

	namespace StringUtils
	{
		NO_DISCARD RLS_API String ConvertFromWide(const WideString& input) noexcept;
		NO_DISCARD RLS_API WideString ConvertToWide(const String& input) noexcept;
		
		NO_DISCARD RLS_API String ExtractTrailingDigits(const String& input) noexcept;
		NO_DISCARD RLS_API std::optional<int> ExtractTrailingNumber(const String& input) noexcept;

		RLS_API void ReplaceCharacters(String& aInputString, char aToReplace, char aToReplaceWith) noexcept;

		NO_DISCARD RLS_API std::vector<String> Split(const String& input, char delimiter) noexcept;
		NO_DISCARD RLS_API String StripTrailingDigits(const std::string& input) noexcept;
		
		NO_DISCARD RLS_API String ToLower(const String& input) noexcept;

		template<typename T>
		requires std::is_integral_v<T>
		NO_DISCARD __forceinline WideString ToWideString(T aValue) noexcept
		{
			return std::to_wstring(aValue);
		}
	}
}