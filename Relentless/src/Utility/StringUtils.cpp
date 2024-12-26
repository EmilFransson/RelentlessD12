#include "StringUtils.h"

namespace Relentless
{
	namespace StringUtils
	{
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