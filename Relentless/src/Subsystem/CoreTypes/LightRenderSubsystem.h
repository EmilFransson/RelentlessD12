#pragma once
#include "Subsystem/ISubsystem.h"

#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RenderProxy/LightRenderProxy.h"
#include "Graphics/RHI/Texture.h"
#include "Graphics/Shaders/Interop/ShaderInterop.h"

namespace Relentless
{
	class Buffer;
	class RenderScene;

	class RLS_API LightRenderSubsystem : public ISubsystem
	{
	public:
		inline static constexpr uint32 MAX_SHADOW_MAP_RESOLUTION = 8192u;
		inline static constexpr uint32 DEFAULT_POSITIONAL_SHADOW_MAP_RESOLUTION = 512u;
		inline static constexpr uint32 DEFAULT_DIRECTIONAL_SHADOW_MAP_RESOLUTION = 2048u;

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
		void BuildDirectionalCascades(const RenderView& aRenderView, const LightRenderProxy& aLightRenderProxy, const std::vector<float>& someCascadeSplits) noexcept;
		void BuildLightData(ShaderInterop::Light& outLightData, const LightRenderProxy& aRenderProxy) const noexcept;
		void BuildPointLightShadowView(const LightRenderProxy& aLightRenderProxy) noexcept;
		void BuildSpotLightShadowView(const LightRenderProxy& aLightRenderProxy) noexcept;
		void BuildShadowViewData(ShaderInterop::ShadowViewData& outShadowViewData, const ShadowView& aShadowView) const noexcept;

		NO_DISCARD std::vector<float> ComputeCascadeSplits(const RenderView& aRenderView, uint32 aNumCascades, float aCascadeDistribution) const noexcept;
		void CreateAndAddShadowView(const LightRenderProxy& aLightRenderProxy, const Matrix& aWorldToClipMatrix, uint32 aResolution, float aTexelWorldSize, uint32 aShadowMapIndex) noexcept;

		void OnUpload(CommandContext& aCommandContext) noexcept;
		void OnViewPrepare(RenderView& aRenderView) noexcept;
	private:
		struct RenderData
		{
			LightRenderProxy RenderProxy;
			std::vector<Ref<Texture>> ShadowMaps;
		};

		std::unordered_map<uint32, RenderData> m_RenderData;
		std::vector<ShaderInterop::Light> m_LightCache;
		std::vector<ShaderInterop::ShadowViewData> m_ShadowViewDataCache;
		std::vector<ShadowView> m_ShadowViews;

		SceneBuffer m_LightDataBuffer;
		SceneBuffer m_ShadowViewDataBuffer;

		CallbackID m_OnUploadCallbackID = INVALID_CALLBACK_ID;
		RenderScene* m_pRenderScene = nullptr;
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}
