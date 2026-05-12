#pragma once
#include <Relentless.h>

namespace Relentless
{
	struct MaterialDetailsContext
	{
		AssetHandle MaterialHandle = AssetHandle::INVALID;
		Ref<Material> Material = nullptr;
	};
}