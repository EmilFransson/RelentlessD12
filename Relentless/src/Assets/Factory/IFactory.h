#pragma once
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class IFactory : public RefCounted<IFactory>
	{
	public:
		virtual ~IFactory() = default;
		virtual void Execute(const Path& filePath, GraphicsDevice* pDevice) noexcept = 0;
	};
}