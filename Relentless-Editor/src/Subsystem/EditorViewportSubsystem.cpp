#include "EditorViewportSubsystem.h"

#include "Core/Editor.h"

#include "Module/ModuleManager.h"
#include "Module/UIModule.h"

#include "Panels/EditorViewportPanel.h"

#include "Subsystem/EditorRendererBridgeSubsystem.h"
#include "Subsystem/EditorSceneBridgeSubsystem.h"
#include "Subsystem/EngineContentSubsystem.h"
#include "Subsystem/SelectionSubsystem.h"

#include "UI/DragDrop/AssetViewDragDropOperation.h"

namespace Relentless
{
	const std::vector<ViewportPanel*>& EditorViewportSubsystem::GetViewportPanels() const noexcept
	{
		return m_EditorViewports;
	}

	bool EditorViewportSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		m_pEditor = static_cast<Editor*>(aSystemManager);
		m_OnUpdateCallbackID = m_pEditor->RegisterUpdateCallback(Callback<void(float)>::Bind(this, &EditorViewportSubsystem::OnUpdate));
		
		UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
		uiModule.OnPanelOpen.Connect(this, &EditorViewportSubsystem::OnPanelOpen);
		uiModule.OnPanelClose.Connect(this, &EditorViewportSubsystem::OnPanelClose);

		return true;
	}

	bool EditorViewportSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

	void EditorViewportSubsystem::ConditionallyDragEntities() noexcept
	{
		Scene* pActiveScene = m_pEditor->GetActiveScene();
		if (!pActiveScene)
			return;

		if (m_DraggedEntities.empty() || !m_pDragContextPanel)
			return;

		const PerspectiveCamera& camera = m_pDragContextPanel->GetClient().GetCamera();
		const ViewTransform& viewTransform = camera.GetViewTransform();

		const FloatRect& viewport = viewTransform.Viewport;
		const float viewportWidth = viewport.GetWidth();
		const float viewportHeight = viewport.GetHeight();

		const Matrix inverseViewMatrix = viewTransform.WorldToView.Invert();
		const Matrix inverseProjectionMatrix = viewTransform.ViewToClip.Invert();
		const Vector2i hoverCoords = m_pDragContextPanel->GetClientHoverCoordinates();

		const Math::Ray ray = Math::ScreenPointToRay(Vector2(hoverCoords.x, hoverCoords.y), Vector2(viewportWidth, viewportHeight), inverseViewMatrix, inverseProjectionMatrix);

		const float offset = 5.0f;
		const Vector3 newWorldLocation = ray.Origin + ray.Direction * offset;

		EntityManager& entityManager = pActiveScene->GetEntityManager();

		std::ranges::for_each(m_DraggedEntities, [&entityManager, &newWorldLocation](entity aEntity)
			{
				entityManager.Get<TransformComponent>(aEntity).SetWorldLocation(newWorldLocation);
			});
	}

	void EditorViewportSubsystem::OnCanvasDragEnter(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation, EditorViewportPanel* aPanel) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return;

		Scene* pActiveScene = m_pEditor->GetActiveScene();
		if (!pActiveScene)
			return;

		AssetViewDragDropOperation& assetViewDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
		const std::vector<AssetData>& assetDatas = assetViewDragDropOperation.GetAssets();
		
		std::vector<AssetData> meshAssetDatas;
		meshAssetDatas.reserve(assetDatas.size());

		for (const AssetData& assetData : assetDatas)
		{
			if (assetData.Type == Mesh::StaticType())
				meshAssetDatas.push_back(assetData);
		}

		if (meshAssetDatas.empty())
			return;

		const PerspectiveCamera& camera = aPanel->GetClient().GetCamera();
		const ViewTransform& viewTransform = camera.GetViewTransform();

		const FloatRect& viewport = viewTransform.Viewport;
		const float viewportWidth = viewport.GetWidth();
		const float viewportHeight = viewport.GetHeight();

		const Matrix inverseViewMatrix = viewTransform.WorldToView.Invert();
		const Matrix inverseProjectionMatrix = viewTransform.ViewToClip.Invert();
		const Vector2i hoverCoords = aPanel->GetClientHoverCoordinates();
		
		const Math::Ray ray = Math::ScreenPointToRay(Vector2(hoverCoords.x, hoverCoords.y), Vector2(viewportWidth, viewportHeight), inverseViewMatrix, inverseProjectionMatrix);

		const float spawnOffset = 5.0f;
		const Vector3 spawnLocation = ray.Origin + ray.Direction * spawnOffset;

		EntityManager& entityManager = pActiveScene->GetEntityManager();
		EngineContentSubsystem& contentSubsystem = *m_pEditor->GetSubsystem<EngineContentSubsystem>();

		m_DraggedEntities.clear();
		m_DraggedEntities.reserve(meshAssetDatas.size());
		m_pDragContextPanel = aPanel;

		for (const AssetData& meshAssetData : meshAssetDatas)
		{
			const AssetHandle meshHandle = AssetManager::LoadAsset(meshAssetData);
			if (!meshHandle.IsValid())
			{
				RLS_CORE_WARN("[EditorViewportSubsystem::OnDropOnCanvas]: Failed to load mesh asset '{0}'.", meshAssetData.Name);
				continue;
			}

			Ref<Mesh> pMesh = AssetManager::Get<Mesh>(meshHandle);
			AssetHandle materialHandle = pMesh->GetDefaultMaterialHandle();
			if (materialHandle.IsValid() && !AssetManager::LoadAsset(materialHandle))
			{
				RLS_CORE_WARN("[EditorViewportSubsystem::OnDropOnCanvas]: Failed to load material asset dependency for mesh '{0}'.", meshAssetData.Name);
				materialHandle = contentSubsystem.GetAssetHandle(EEngineAsset::WhiteMaterial);
			}
			else if (!materialHandle.IsValid())
				materialHandle = contentSubsystem.GetAssetHandle(EEngineAsset::WhiteMaterial);

			const entity newEntity = pActiveScene->CreateEntity(pMesh->GetName().c_str());
			entityManager.Get<TransformComponent>(newEntity).SetWorldLocation(spawnLocation);
			entityManager.Add<MeshFilterComponent>(newEntity).SetMesh(meshHandle);
			entityManager.Add<MeshRendererComponent>(newEntity).SetMaterial(materialHandle);

			m_DraggedEntities.push_back(newEntity);
		}
	}

	void EditorViewportSubsystem::OnCanvasDragLeave(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation, EditorViewportPanel* aPanel) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return;

		if (m_pDragContextPanel != aPanel)
			return;

		Scene* pActiveScene = m_pEditor->GetActiveScene();
		if (!pActiveScene)
			return;

		std::ranges::for_each(m_DraggedEntities, [pActiveScene](entity aEntity)
			{
				pActiveScene->DestroyEntity(aEntity);
			});

		m_DraggedEntities.clear();
		m_pDragContextPanel = nullptr;
	}

	Reply EditorViewportSubsystem::OnCanvasDragOver(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return Reply::Handled();

		return Reply::Unhandled();
	}

	Reply EditorViewportSubsystem::OnDropOnCanvas(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return Reply::Unhandled();

		if (!m_DraggedEntities.empty())
		{
			SelectionSubsystem* pSelectionSubsystem = m_pEditor->GetSubsystem<SelectionSubsystem>();
			pSelectionSubsystem->SelectEntities(m_DraggedEntities);
			m_DraggedEntities.clear();
		}
		m_pDragContextPanel = nullptr;

		AssetViewDragDropOperation& assetViewDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
		const std::vector<AssetData>& assetDatas = assetViewDragDropOperation.GetAssets();
		
		std::vector<AssetData> materialAssetDatas;
		std::vector<AssetData> environmentAssetDatas;

		for (const AssetData& assetData : assetDatas)
		{
			if (assetData.Type == Material::StaticType())
				materialAssetDatas.push_back(assetData);
			else if (assetData.Type == Environment::StaticType())
				environmentAssetDatas.push_back(assetData);
		}

		if (materialAssetDatas.empty() && environmentAssetDatas.empty())
			return Reply::Unhandled();

		Scene* pActiveScene = m_pEditor->GetActiveScene();
		if (!pActiveScene)
			return Reply::Unhandled();

		EntityManager& entityManager = pActiveScene->GetEntityManager();

		EditorRendererBridgeSubsystem& editorRendererBridge = *m_pEditor->GetSubsystem<EditorRendererBridgeSubsystem>();
		const entity hoveredEntity = editorRendererBridge.GetHoveredEntity();
		
		if (hoveredEntity != NULL_ENTITY && !materialAssetDatas.empty())
		{
			const AssetHandle materialHandle = AssetManager::LoadAsset(materialAssetDatas.back());

			if (materialHandle.IsValid())
				entityManager.Get<MeshRendererComponent>(hoveredEntity).SetMaterial(materialHandle);
		}

		if (hoveredEntity == NULL_ENTITY && !environmentAssetDatas.empty())
		{
			const AssetHandle environmentHandle = AssetManager::LoadAsset(environmentAssetDatas.back());
			
			if (const entity skyBoxEntity = pActiveScene->GetActiveSkyBox(); skyBoxEntity != NULL_ENTITY)
				entityManager.Get<SkyBoxComponent>(skyBoxEntity).SetPrimaryEnvironment(environmentHandle);
			if (const entity skyLightEntity = pActiveScene->GetActiveSkyLight(); skyLightEntity != NULL_ENTITY)
				entityManager.Get<SkyLightComponent>(skyLightEntity).SetPrimaryEnvironment(environmentHandle);
		}

		return Reply::Handled();
	}

	void EditorViewportSubsystem::OnPanelClose(PanelBase* aPanel) noexcept
	{
		if (ViewportPanel* pViewportPanel = dynamic_cast<ViewportPanel*>(aPanel))
		{
			pViewportPanel->OnClickedOnViewport.Detach(this);
			pViewportPanel->OnHotkeyPressed.Detach(this);

			if (EditorViewportPanel* pEditorViewportPanel = dynamic_cast<EditorViewportPanel*>(pViewportPanel))
			{
				pEditorViewportPanel->OnCanvasDragEnter.Detach(m_CanvasDragEnterCallbackIDs[pEditorViewportPanel]);
				m_CanvasDragEnterCallbackIDs.erase(pEditorViewportPanel);

				pEditorViewportPanel->OnCanvasDragLeave.Detach(m_CanvasDragLeaveCallbackIDs[pEditorViewportPanel]);
				m_CanvasDragLeaveCallbackIDs.erase(pEditorViewportPanel);

				pEditorViewportPanel->OnCanvasDragOver.Detach(this);
				pEditorViewportPanel->OnCanvasDrop.Detach(this);

				if (m_pDragContextPanel == pEditorViewportPanel)
					m_pDragContextPanel = nullptr;
			}

			std::erase_if(m_EditorViewports, [pViewportPanel](ViewportPanel* aViewportPanel) { return aViewportPanel == pViewportPanel; });
		}
	}

	void EditorViewportSubsystem::OnPanelOpen(PanelBase* aPanel) noexcept
	{
		if (ViewportPanel* pViewportPanel = dynamic_cast<ViewportPanel*>(aPanel))
		{
			pViewportPanel->OnClickedOnViewport.Connect(this, &EditorViewportSubsystem::OnViewportClicked);
			pViewportPanel->OnHotkeyPressed.Connect(this, &EditorViewportSubsystem::OnViewportHotkeyPressed);

			if (EditorViewportPanel* pEditorViewportPanel = dynamic_cast<EditorViewportPanel*>(pViewportPanel))
			{
				m_CanvasDragEnterCallbackIDs[pEditorViewportPanel] = pEditorViewportPanel->OnCanvasDragEnter.Connect([this, pEditorViewportPanel](const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation)
					{
						OnCanvasDragEnter(aWidgetGeometry, aDragDropOperation, pEditorViewportPanel);
					});

				m_CanvasDragLeaveCallbackIDs[pEditorViewportPanel] = pEditorViewportPanel->OnCanvasDragLeave.Connect([this, pEditorViewportPanel](const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation)
					{
						OnCanvasDragLeave(aWidgetGeometry, aDragDropOperation, pEditorViewportPanel);
					});
				
				pEditorViewportPanel->OnCanvasDragOver.Connect(this, &EditorViewportSubsystem::OnCanvasDragOver);
				pEditorViewportPanel->OnCanvasDrop.Connect(this, &EditorViewportSubsystem::OnDropOnCanvas);
			}

			m_EditorViewports.push_back(pViewportPanel);
		}
	}

	void EditorViewportSubsystem::OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept
	{
		ConditionallyDragEntities();

		std::vector<ViewRenderDesc> viewRenderDescs(m_EditorViewports.size());
		for (size_t i = 0; i < m_EditorViewports.size(); ++i)
			viewRenderDescs[i] = m_EditorViewports[i]->BuildRenderDescriptor();

		if (viewRenderDescs.empty())
			return;

		Renderer::Dispatch([renderDescs = std::move(viewRenderDescs)](Renderer* aRenderer) 
			{ 
				aRenderer->RenderViews(renderDescs); 
			});
	}

	void EditorViewportSubsystem::OnViewportClicked(MAYBE_UNUSED ViewportPanel* aPanel, MAYBE_UNUSED Vector2u aRelativeMouseCoords) noexcept
	{
		SelectionSubsystem* pSelection = m_pEditor->GetSubsystem<SelectionSubsystem>();
		EditorRendererBridgeSubsystem* pEditorRendererBridge = m_pEditor->GetSubsystem<EditorRendererBridgeSubsystem>();

		const entity hoveredEntity = pEditorRendererBridge->GetHoveredEntity();
		const bool lCtrlDown = Keyboard::IsKeyDown(RLS_Key::LCtrl);
		const bool lShiftDown = Keyboard::IsKeyDown(RLS_Key::LShift);
		const bool isHoveringEntity = hoveredEntity != NULL_ENTITY;

		if (!isHoveringEntity || (!lCtrlDown && !lShiftDown))
			pSelection->DeselectAllEntities();

		if (isHoveringEntity)
		{
			if (lCtrlDown && pSelection->IsEntitySelected(hoveredEntity))
				pSelection->DeselectEntity(hoveredEntity);
			else
				pSelection->SelectEntity(hoveredEntity);
		}
	}

	void EditorViewportSubsystem::OnViewportHotkeyPressed(MAYBE_UNUSED ViewportPanel* aPanel, RLS_Key aKey) noexcept
	{
		switch (aKey)
		{
		case RLS_Key::A:
			if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
				m_pEditor->GetSubsystem<EditorSceneBridgeSubsystem>()->SelectAllEntities();
			break;
		case RLS_Key::H:
			m_pEditor->GetSubsystem<EditorSceneBridgeSubsystem>()->SetVisibilityForSelectedEntities(Keyboard::IsKeyDown(RLS_Key::LCtrl));
			break;
		case RLS_Key::Delete:
			m_pEditor->GetSubsystem<EditorSceneBridgeSubsystem>()->DeleteSelectedEntities();
			break;
		default:
			break;
		}
	}
}