#pragma once
#include "ECS/Component.h"

#include "Graphics/Renderer/RenderTypes.h"

namespace Relentless
{
	class Material;

	struct RLS_API MeshRendererComponent : public ManagedComponent<MeshRendererComponent>
	{
	public:
		MeshRendererComponent() = default;
		MeshRendererComponent(MeshRendererComponent&&) noexcept = default;
		MeshRendererComponent& operator=(MeshRendererComponent&&) noexcept = default;
		virtual ~MeshRendererComponent() noexcept override;

		struct DirtyRenderState{};

		void CopyFrom(const MeshRendererComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override;

		NO_DISCARD ELightChannel GetLightChannels() const noexcept;
		NO_DISCARD Ref<Material> GetMaterial() const noexcept;
		NO_DISCARD const AssetHandle& GetMaterialHandle() const noexcept;

		NO_DISCARD bool HasLightChannelsEnabled(ELightChannel aChannels) const noexcept;
		NO_DISCARD bool HasAssignedMaterial() const noexcept;

		NO_DISCARD bool IsCastingShadows() const noexcept;

		void OnBound() noexcept override;
		
		void RemoveMaterial() noexcept;

		void SetCastShadows(bool aCastShadows) noexcept;
		void SetLightChannelEnabled(ELightChannel aChannel, bool aEnabled) noexcept;
		void SetMaterial(const AssetHandle& aAssetHandle) noexcept;
	private:
		void ApplyChannelMask(ELightChannel aNewMask) noexcept;
		
		void ConnectMaterial() noexcept;

		void DetachMaterial() noexcept;

		void OnMaterialAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept;
		void OnMaterialAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
	private:
		AssetHandle m_MaterialHandle = AssetHandle::INVALID;
		ELightChannel m_LightChannels = ELightChannel::Default;
		bool m_CastShadows = true;

		CallbackID m_MaterialDestroyCallbackID = INVALID_CALLBACK_ID;
		CallbackID m_MaterialChangedCallbackID = INVALID_CALLBACK_ID;
	};
}
