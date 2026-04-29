#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/LightRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class RLS_API LightRenderSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD uint32 GetNumLights() const noexcept;
		NO_DISCARD const Buffer* GetRenderData() const;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<LightRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<uint32> someIDs) noexcept;
	private:
		void BuildLightData(ShaderInterop::Light& outLightData, const LightRenderProxy& aRenderProxy) const noexcept;

		void OnUpload(CommandContext& aCommandContext) noexcept;
	private:
		std::unordered_map<uint32, LightRenderProxy> m_RenderData;
		std::vector<ShaderInterop::Light> m_LightCache;

		SceneBuffer m_LightDataBuffer;

		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
