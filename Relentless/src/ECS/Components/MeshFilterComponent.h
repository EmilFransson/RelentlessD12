#pragma once
#include "ECS/Component.h"

namespace Relentless
{
	class Mesh;

	struct RLS_API MeshFilterComponent : public ManagedComponent<MeshFilterComponent>
	{
	public:
		MeshFilterComponent() = default;
		MeshFilterComponent(MeshFilterComponent&&) noexcept;
		MeshFilterComponent& operator=(MeshFilterComponent&&) noexcept;
		virtual ~MeshFilterComponent() noexcept override;

		struct DirtyRenderState {};

		void CopyFrom(const MeshFilterComponent& aOtherComponent, entity aThisEntity, EntityManager& aEntityManager) override;
		
		NO_DISCARD Ref<Mesh> GetMesh() const noexcept;
		NO_DISCARD const AssetHandle& GetMeshHandle() const noexcept;

		NO_DISCARD bool HasAssignedMesh() const noexcept;

		void OnBound() noexcept override;
		
		void RemoveMesh() noexcept;

		void SetMesh(const AssetHandle& aAssetHandle) noexcept;
	private:
		void ConnectMesh() noexcept;

		void DetachMesh() noexcept;

		void OnMeshAssetDestroy(MAYBE_UNUSED IAsset* aAsset) noexcept;
		void OnMeshAssetPropertyChanged(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept;
	private:
		AssetHandle m_MeshHandle = NULL_HANDLE;
	};
}