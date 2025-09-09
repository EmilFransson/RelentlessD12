#include "StringUtils.h"

namespace Relentless
{
	namespace StringUtils
	{
		std::string ConvertFromWide(const std::wstring& input) noexcept
		{
			const int size = ::WideCharToMultiByte(CP_ACP, 0, input.data(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);
			RLS_ASSERT(size > 0, "WideCharToMultiByte conversion failed.");

			std::string result(size, '\0');
			::WideCharToMultiByte(CP_ACP, 0, input.data(), static_cast<int>(input.size()), result.data(), size, nullptr, nullptr);

			return result;
		}

		std::wstring ConvertToWide(const String& input) noexcept
		{
			const int size = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, input.data(), static_cast<int>(input.size()), nullptr, 0);
			RLS_ASSERT(size > 0, "MultiByteToWideChar conversion failed.");

			std::wstring result(size, L'\0');
			::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, input.data(), static_cast<int>(input.size()), result.data(), size);

			return result;
		}

		String ExtractTrailingDigits(const String& input) noexcept
		{
			size_t pos = input.size();
			while (pos > 0 && std::isdigit(static_cast<unsigned char>(input[pos - 1])))
				--pos;

			return input.substr(pos);
		}

		std::optional<int> ExtractTrailingNumber(const String& input) noexcept
		{
			auto digits = ExtractTrailingDigits(input);
			if (digits.empty()) 
				return std::nullopt;

			return std::stoi(digits);
		}

		std::vector<String> Split(const String& input, char delimiter) noexcept
		{
			std::vector<String> tokens;

			size_t start = 0;
			size_t end = input.find(delimiter);

			while (start < input.size() && input[start] == delimiter)
				start++;

			while (end != String::npos)
			{
				if (start < end)
					tokens.push_back(input.substr(start, end - start));

				start = end + 1;

				while (start < input.size() && input[start] == delimiter)
					start++;

				end = input.find(delimiter, start);
			}

			if (start < input.size())
				tokens.push_back(input.substr(start));

			return tokens;
		}

		String StripTrailingDigits(const std::string& input) noexcept
		{
			size_t pos = input.size();
			while (pos > 0 && std::isdigit(static_cast<unsigned char>(input[pos - 1])))
			{
				--pos;
			}
			return input.substr(0, pos);
		}

		String ToLower(const String& input) noexcept
		{
			String result = input;
			std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
			
			return result;
		}
	}
}