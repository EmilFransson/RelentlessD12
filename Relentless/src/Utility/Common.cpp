#include "Common.h"

#include "Graphics/RHI/D3D.h"

#include "StringUtils.h"
namespace Relentless
{
	UUID ConvertStringToGUID(const std::string& guidString) noexcept
	{
		const std::wstring GUIDAsWString = StringUtils::ConvertToWide(guidString);
		GUID uuid{};
		VERIFY_HR(IIDFromString(GUIDAsWString.c_str(), &uuid));
		return uuid;
	}

	std::string ConvertUUIDToString(const UUID& uuid) noexcept
	{
		wchar_t wszUuid[39] = { 0 };
		std::string strUuid = {};

		if (::StringFromGUID2(uuid, wszUuid, 39) > 0)
		{
			char chUuid[39];
			::WideCharToMultiByte(CP_ACP, 0, wszUuid, -1, chUuid, 39, NULL, NULL);

			strUuid = chUuid;
		}

		return strUuid;
	}

	UUID CreateUUID() noexcept
	{
		UUID uuID;
#if defined RLS_DEBUG
		RLS_ASSERT(::UuidCreate(&uuID) == RPC_S_OK, "Failed to generate UUID.");
#else
		::UuidCreate(&uuID);
#endif

		return uuID;
	}
}