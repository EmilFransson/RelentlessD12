#pragma once

#include "Core/DLLExport.h"

#include "ECSCommon.h"
#include "ECS/Components/LightComponent.h"

namespace Relentless::EntityUtils
{
	RLS_API void ConvertLightType(entity aEntity, EntityManager& aEntityManager, ELightType aFromLightType, ELightType aToLightType) noexcept;
}