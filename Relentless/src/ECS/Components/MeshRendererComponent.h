#pragma once
#include "ECS/Component.h"

namespace Relentless
{
	class Material;

	struct RLS_API MeshRendererComponent : public ManagedComponent<MeshRendererComponent>
	{
	public:
		MeshRendererComponent() = default;
		MeshRendererComponent(MeshRendererComponent&&) noexcept;
		MeshRendererComponent& operator=(MeshRendererComponent&&) noexcept;
		virtual ~MeshRendererComponent() noexcept override;

		struct DirtyRenderState{};

		void CopyFrom(const MeshRendererComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override;

		NO_DISCARD Ref<Material> GetMaterial() const noexcept;
		NO_DISCARD const AssetHandle& GetMaterialHandle() const noexcept;

		NO_DISCARD bool HasAssignedMaterial() const noexcept;

		void OnBound() noexcept override;
		
		void RemoveMaterial() noexcept;

		void SetMaterial(const AssetHandle& aAssetHandle) noexcept;
	private:
		void ConnectMaterial() noexcept;

		void DetachMaterial() noexcept;

		void OnMaterialAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept;
		void OnMaterialAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
	private:
		AssetHandle m_MaterialHandle = NULL_HANDLE;
	};
}
