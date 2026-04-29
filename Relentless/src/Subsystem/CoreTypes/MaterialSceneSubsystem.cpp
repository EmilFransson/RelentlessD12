#include "MaterialSceneSubsystem.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Material.h"
#include "Assets/CoreTypes/Texture2D.h"

#include "Graphics/Renderer/Renderer.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/MaterialRenderSubsystem.h"

namespace Relentless
{
	bool MaterialSceneSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		m_pScene = static_cast<Scene*>(aSystemManager);

		std::vector<MaterialRenderProxy> renderProxies;
		renderProxies.reserve(AssetManager::GetNumAssets<Material>());

		auto lock = AssetManager::LockStorage<Material>();

		AssetManager::ConnectOnAssetCreated<Material>(lock, this, &MaterialSceneSubsystem::OnMaterialAssetCreated);

		AssetManager::ForEachAsset<Material>(lock, [this, &renderProxies](Material& aMaterial)
			{
				renderProxies.push_back(CreateRenderProxy(aMaterial));
				aMaterial.OnPropertyChanged.Connect(this, &MaterialSceneSubsystem::OnMaterialAssetEdited);
				aMaterial.OnDestroy.Connect(this, &MaterialSceneSubsystem::OnMaterialAssetDestroy);

				return true;
			});

		if (renderProxies.empty())
			return true;

		Renderer::Dispatch([proxies = std::move(renderProxies), uid = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				MaterialRenderSubsystem* pMaterialRenderSubsystem = pRenderScene->GetSubsystem<MaterialRenderSubsystem>();
				pMaterialRenderSubsystem->Patch(std::move(proxies));
			});

		return true;
	}

	void MaterialSceneSubsystem::OnUnload(ISystemManager* aSystemManager) noexcept
	{
		AssetManager::DetachOnAssetCreated<Material>(this);
	}

	bool MaterialSceneSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Scene*>(aSystemManager) != nullptr;
	}

	MaterialRenderProxy MaterialSceneSubsystem::CreateRenderProxy(const Material& aMaterial) noexcept
	{
		auto FetchTexture = [&aMaterial](ETextureType aType) -> Ref<Texture>
			{
				if (aMaterial.HasTexture(aType))
				{
					Ref<Texture2D> pTexture2D = aMaterial.GetTexture(aType);
					if (!pTexture2D->GetResource())
						pTexture2D->CreateResource();

					return pTexture2D->GetResource();
				}

				return nullptr;
			};

		MaterialRenderProxy materialRenderProxy{};
		materialRenderProxy.ID = aMaterial.GetUUID();
		materialRenderProxy.AlbedoColor = aMaterial.GetAlbedoColor();
		materialRenderProxy.EmissiveColor = aMaterial.GetEmissiveColor();
		materialRenderProxy.TilingFactor = aMaterial.GetGlobalTilingFactor();
		materialRenderProxy.Offset = aMaterial.GetGlobalOffset();
		materialRenderProxy.Metallic = aMaterial.GetMetalness();
		materialRenderProxy.EmissionIntensity = aMaterial.GetEmissiveIntensity();
		materialRenderProxy.Roughness = aMaterial.GetRoughness();
		materialRenderProxy.DisplacementIntensity = aMaterial.GetDisplacementIntensity();
		materialRenderProxy.AmbientOcclusionIntensity = aMaterial.GetAmbientOcclusionIntensity();
		materialRenderProxy.BlendMode = aMaterial.GetBlendMode();
		materialRenderProxy.IsTwoSided = aMaterial.IsTwoSided();

		materialRenderProxy.AlbedoMap = FetchTexture(ETextureType::Albedo);
		materialRenderProxy.NormalMap = FetchTexture(ETextureType::NormalMap);
		materialRenderProxy.DisplacementMap = FetchTexture(ETextureType::DisplacementMap); 
		materialRenderProxy.RoughnessMap = FetchTexture(ETextureType::Roughness);
		materialRenderProxy.MetalnessMap = FetchTexture(ETextureType::Metallic);
		materialRenderProxy.EmissionMap = FetchTexture(ETextureType::Emission);
		materialRenderProxy.AmbientOcclusionMap = FetchTexture(ETextureType::AmbientOcclusion);

		return materialRenderProxy;
	}

	void MaterialSceneSubsystem::OnMaterialAssetCreated(const AssetHandle& aMaterialHandle) noexcept
	{
		Ref<Material> pMaterial = AssetManager::Get<Material>(aMaterialHandle);

		std::vector<MaterialRenderProxy> renderProxies;
		renderProxies.push_back(CreateRenderProxy(*pMaterial));

		Renderer::Dispatch([proxies = std::move(renderProxies), uid = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				MaterialRenderSubsystem* pMaterialRenderSubsystem = pRenderScene->GetSubsystem<MaterialRenderSubsystem>();
				pMaterialRenderSubsystem->Patch(std::move(proxies));
			});
	}

	void MaterialSceneSubsystem::OnMaterialAssetDestroy(IAsset* aMaterialAsset) noexcept
	{
		Material* pMaterial = static_cast<Material*>(aMaterialAsset);

		Renderer::Dispatch([materialID = pMaterial->GetUUID(), sceneID = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(sceneID);
				MaterialRenderSubsystem* pMaterialRenderSubsystem = pRenderScene->GetSubsystem<MaterialRenderSubsystem>();
				pMaterialRenderSubsystem->Remove({ materialID });
			});
	}

	void MaterialSceneSubsystem::OnMaterialAssetEdited(IAsset* aMaterialAsset, MAYBE_UNUSED uint64 aEditedProperty) noexcept
	{
		Material* pMaterial = static_cast<Material*>(aMaterialAsset);

		std::vector<MaterialRenderProxy> renderProxies;
		renderProxies.push_back(CreateRenderProxy(*pMaterial));

		Renderer::Dispatch([proxies = std::move(renderProxies), uid = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				MaterialRenderSubsystem* pMaterialRenderSubsystem = pRenderScene->GetSubsystem<MaterialRenderSubsystem>();
				pMaterialRenderSubsystem->Patch(std::move(proxies));
			});
	}
}