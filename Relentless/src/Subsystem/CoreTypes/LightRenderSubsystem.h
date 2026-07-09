#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/LightRenderProxy.h"
#include "Graphics/RHI/Buffer.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class RenderScene;

	class RLS_API LightRenderSubsystem : public ISubsystem
	{
	public:
		static constexpr uint32 MAX_CASCADES = 4u;

		NO_DISCARD uint32 GetNumLights() const noexcept;
		NO_DISCARD const Buffer* GetLightRenderData() const noexcept;
		NO_DISCARD const std::vector<ShadowView>& GetShadowViews() const noexcept;
		NO_DISCARD const Buffer* GetShadowViewsRenderData() const noexcept;

		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;

		void Patch(std::vector<LightRenderProxy> someRenderProxyUpdates) noexcept;

		void Remove(std::vector<uint32> someIDs) noexcept;
	private:
		void BuildDirectionalCascades(const RenderView& aRenderView, const LightRenderProxy& aLightRenderProxy, const std::array<float, MAX_CASCADES>& someCascadeSplits, uint32& aShadowViewIndex) noexcept;
		void BuildLightData(ShaderInterop::Light& outLightData, const LightRenderProxy& aRenderProxy) const noexcept;
		void BuildShadowViewData(ShaderInterop::ShadowViewData& outShadowViewData, const ShadowView& aShadowView) const noexcept;

		NO_DISCARD std::array<float, MAX_CASCADES> ComputeCascadeSplits(const RenderView& aRenderView) const noexcept;

		void OnUpload(CommandContext& aCommandContext) noexcept;
		void OnViewPrepare(RenderView& aRenderView) noexcept;
	private:
		std::unordered_map<uint32, LightRenderProxy> m_RenderData;
		std::vector<ShaderInterop::Light> m_LightCache;
		std::vector<ShaderInterop::ShadowViewData> m_ShadowViewDataCache;

		std::vector<Ref<Texture>> m_ShadowMaps;
		std::vector<ShadowView> m_ShadowViews;

		SceneBuffer m_LightDataBuffer;
		SceneBuffer m_ShadowViewDataBuffer;

		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;
		RenderScene* m_pRenderScene = nullptr;
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
