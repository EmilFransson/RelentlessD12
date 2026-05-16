#include "MeshSceneSubsystem.h"

#include "Assets/AssetManager.h"
#include "Assets/CoreTypes/Mesh.h"

#include "Graphics/Renderer/Renderer.h"

#include "Scene/Scene.h"
#include "Subsystem/CoreTypes/MeshRenderSubsystem.h"

namespace Relentless
{
	bool MeshSceneSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		m_pScene = static_cast<Scene*>(aSystemManager);

		std::vector<MeshRenderProxy> renderProxies;
		renderProxies.reserve(AssetManager::GetNumAssets<Mesh>());

		auto lock = AssetManager::LockStorage<Mesh>();

		AssetManager::ConnectOnAssetCreated<Mesh>(lock, this, &MeshSceneSubsystem::OnMeshAssetCreated);

		AssetManager::ForEachAsset<Mesh>(lock, [this, &renderProxies](Mesh& aMesh)
			{
				renderProxies.push_back(CreateRenderProxy(aMesh));
				aMesh.OnPropertyChanged.Connect(this, &MeshSceneSubsystem::OnMeshAssetEdited);
				aMesh.OnDestroy.Connect(this, &MeshSceneSubsystem::OnMeshAssetDestroy);

				return true;
			});

		if (renderProxies.empty())
			return true;

		Renderer::Dispatch([proxies = std::move(renderProxies), uid = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				MeshRenderSubsystem* pMeshRenderSubsystem = pRenderScene->GetSubsystem<MeshRenderSubsystem>();
				pMeshRenderSubsystem->Patch(std::move(proxies));
			});

		return true;
	}

	void MeshSceneSubsystem::OnUnload(MAYBE_UNUSED ISystemManager* aSystemManager) noexcept
	{
		auto lock = AssetManager::LockStorage<Mesh>();

		AssetManager::DetachOnAssetCreated<Mesh>(lock, this);

		AssetManager::ForEachAsset<Mesh>(lock, [this](Mesh& aMesh)
			{
				aMesh.OnPropertyChanged.Detach(this);
				aMesh.OnDestroy.Detach(this);

				return true;
			});
	}

	bool MeshSceneSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Scene*>(aSystemManager) != nullptr;
	}

	MeshRenderProxy MeshSceneSubsystem::CreateRenderProxy(const Mesh& aMesh) noexcept
	{
		MeshRenderProxy meshRenderProxy{};
		meshRenderProxy.ID = aMesh.GetUUID();
		meshRenderProxy.VertexBuffer = aMesh.GetVertexBuffer();
		meshRenderProxy.IndexBuffer = aMesh.GetIndexBuffer();

		return meshRenderProxy;
	}

	void MeshSceneSubsystem::OnMeshAssetCreated(const AssetHandle& aMeshHandle) noexcept
	{
		Ref<Mesh> pMesh = AssetManager::Get<Mesh>(aMeshHandle);
		pMesh->OnPropertyChanged.Connect(this, &MeshSceneSubsystem::OnMeshAssetEdited);
		pMesh->OnDestroy.Connect(this, &MeshSceneSubsystem::OnMeshAssetDestroy);

		std::vector<MeshRenderProxy> renderProxies;
		renderProxies.push_back(CreateRenderProxy(*pMesh));

		Renderer::Dispatch([proxies = std::move(renderProxies), uid = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				MeshRenderSubsystem* pMeshRenderSubsystem = pRenderScene->GetSubsystem<MeshRenderSubsystem>();
				pMeshRenderSubsystem->Patch(std::move(proxies));
			});
	}

	void MeshSceneSubsystem::OnMeshAssetDestroy(IAsset* aMeshAsset) noexcept
	{
		Mesh* pMesh = static_cast<Mesh*>(aMeshAsset);

		Renderer::Dispatch([meshID = pMesh->GetUUID(), sceneID = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(sceneID);
				MeshRenderSubsystem* pMeshRenderSubsystem = pRenderScene->GetSubsystem<MeshRenderSubsystem>();
				pMeshRenderSubsystem->Remove({ meshID });
			});
	}

	void MeshSceneSubsystem::OnMeshAssetEdited(IAsset* aMeshAsset, MAYBE_UNUSED uint64 aEditedProperty) noexcept
	{
		Mesh* pMesh = static_cast<Mesh*>(aMeshAsset);

		std::vector<MeshRenderProxy> renderProxies;
		renderProxies.push_back(CreateRenderProxy(*pMesh));

		Renderer::Dispatch([proxies = std::move(renderProxies), uid = m_pScene->GetUUID()](Renderer* aRenderer)
			{
				RenderScene* pRenderScene = aRenderer->GetRenderScene(uid);
				MeshRenderSubsystem* pMeshRenderSubsystem = pRenderScene->GetSubsystem<MeshRenderSubsystem>();
				pMeshRenderSubsystem->Patch(std::move(proxies));
			});
	}
}