#include "ManagerUtilities.h"
#include "../Core/Utility.h"
namespace Relentless
{
	std::string GetAssetHandleAsString(const AssetHandle& assetHandle) noexcept
	{
		wchar_t assetHandleString[39];
		StringFromGUID2(assetHandle.UUID, assetHandleString, 39);
		return  ConvertWideStringToString(std::wstring(assetHandleString));
	}
}