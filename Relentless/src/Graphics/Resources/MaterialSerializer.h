#pragma once
#include "Material.h"

namespace Relentless
{
	class MaterialSerializer
	{
	public:
		static void Serialize(MaterialHandle& materialHandle, const std::string& fullPath, const std::string& toReplace = "") noexcept;
		static MaterialHandle Deserialize(const std::string& fullPath) noexcept;
	};
}