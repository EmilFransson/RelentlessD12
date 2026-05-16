#pragma once
#include "Subsystem/ISubsystem.h"

namespace Relentless
{
	struct AssetHandle;
	class IAsset;
	class Mesh;
	struct MeshRenderProxy;
	class Scene;

	class MeshSceneSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		NO_DISCARD MeshRenderProxy CreateRenderProxy(const Mesh& aMesh) noexcept;

		void OnMeshAssetCreated(const AssetHandle& aMaterialHandle) noexcept;
		void OnMeshAssetDestroy(IAsset* aMeshAsset) noexcept;
		void OnMeshAssetEdited(IAsset* aMeshAsset, MAYBE_UNUSED uint64 aEditedProperty) noexcept;
	private:
		Scene* m_pScene = nullptr;
	};
}