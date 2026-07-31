#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/ExponentialHeightFogRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class RLS_API FogRenderSubsystem : public ISubsystem
	{
	public:
		inline static const uint32 INVALID_ID = 0xFFFFFFFF;

		NO_DISCARD const Buffer* GetRenderData() const;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<ExponentialHeightFogRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<uint32> someIDs) noexcept;
	private:
		void BuildFogData(ShaderInterop::FogData& outFogData) const noexcept;

		void OnUpload(CommandContext& aCommandContext) noexcept;
	private:
		std::unordered_map<uint32, ExponentialHeightFogRenderProxy> m_RenderData;

		std::vector<ShaderInterop::FogData> m_MaterialCache;

		Ref<Buffer> m_pFogDataBuffer = nullptr;

		uint32 m_ActiveID = INVALID_ID;
		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
