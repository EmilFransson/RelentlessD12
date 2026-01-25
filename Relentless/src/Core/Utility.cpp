#include "Utility.h"
#include "Core.h"
#include "Log.h"
namespace Relentless
{
	String ConvertWideStringToString(WideString const& aWideString) noexcept
	{
		if (aWideString.empty())
			return {};

		const int requiredSize = ::WideCharToMultiByte(CP_UTF8, 0, aWideString.data(), static_cast<int>(aWideString.size()), nullptr, 0, nullptr, nullptr);

		String result(requiredSize, '\0');
		::WideCharToMultiByte(CP_UTF8, 0, aWideString.data(), static_cast<int>(aWideString.size()), result.data(), requiredSize, nullptr, nullptr);

		return result;
	}

	WideString ConvertStringToWideString(const String& aString) noexcept
	{
		const int size = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, aString.c_str(), static_cast<int>(aString.size()), nullptr, 0);
		RLS_ASSERT(size > 0, "MultiByteToWideChar conversion did not succeed.");
		
		WideString converted{};
		converted.resize(size);
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, aString.c_str(), static_cast<int>(aString.size()), &(*converted.begin()), static_cast<int>(converted.size()));

		return converted;
	}
}