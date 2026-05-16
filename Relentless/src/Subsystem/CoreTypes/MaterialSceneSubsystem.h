#pragma once
#include "Subsystem/ISubsystem.h"

namespace Relentless
{
	struct AssetHandle;
	class IAsset;
	class Material;
	struct MaterialRenderProxy;
	class Scene;

	class MaterialSceneSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		void OnUnload(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept override;

		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		NO_DISCARD MaterialRenderProxy CreateRenderProxy(const Material& aMaterial) noexcept;

		void OnMaterialAssetCreated(const AssetHandle& aMaterialHandle) noexcept;
		void OnMaterialAssetDestroy(IAsset* aMaterialAsset) noexcept;
		void OnMaterialAssetEdited(IAsset* aMaterialAsset, MAYBE_UNUSED uint64 aEditedProperty) noexcept;
	private:
		Scene* m_pScene = nullptr;
	};
}