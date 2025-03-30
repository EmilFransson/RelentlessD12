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

		std::wstring ConvertToWide(const std::string& input) noexcept
		{
			const int size = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, input.data(), static_cast<int>(input.size()), nullptr, 0);
			RLS_ASSERT(size > 0, "MultiByteToWideChar conversion failed.");

			std::wstring result(size, L'\0');
			::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, input.data(), static_cast<int>(input.size()), result.data(), size);

			return result;
		}

		std::vector<std::string> Split(const std::string& input, char delimiter) noexcept
		{
			std::vector<std::string> tokens;

			size_t start = 0;
			size_t end = input.find(delimiter);

			while (start < input.size() && input[start] == delimiter)
				start++;

			while (end != std::string::npos)
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

	}
}