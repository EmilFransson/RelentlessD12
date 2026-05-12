#pragma once
#include <Relentless.h>

namespace Relentless
{
	struct EnvironmentDetailsContext
	{
		AssetHandle EnvironmentHandle = AssetHandle::INVALID;
		Ref<Environment> Environment = nullptr;
	};
}