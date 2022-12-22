#include "Utility.h"
#include "Core.h"
#include "Log.h"
namespace Relentless
{
	std::string ConvertWstringToString(std::wstring const& wstr) noexcept
	{
#pragma warning(push, 0)
		std::size_t size = sizeof(wstr);
		char* str = RLS_NEW char[size];
		std::string temp;

		std::wcstombs(str, wstr.c_str(), size);

		temp = str;
		delete[] str;
#pragma warning(pop)
		return temp;
	}
	std::wstring ConvertStringToWstring(const std::string& string) noexcept
	{
		int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, string.c_str(), static_cast<int>(string.size()), nullptr, 0);
		RLS_ASSERT(size > 0, "MultiByteToWideChar conversion did not succeed.");
		std::wstring converted{};
		converted.resize(size);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, string.c_str(), static_cast<int>(string.size()), &(*converted.begin()), static_cast<int>(converted.size()));

		return converted;
	}
}