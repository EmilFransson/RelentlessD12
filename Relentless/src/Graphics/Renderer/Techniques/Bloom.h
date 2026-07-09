#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class Bloom
	{
	public:
		Bloom(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& aCommandContext, SceneTextures& aSceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}