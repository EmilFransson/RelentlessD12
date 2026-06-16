#include "EditorViewportSubsystem.h"

#include "Core/Editor.h"

#include "Module/ModuleManager.h"
#include "Module/UIModule.h"

#include "Panels/EditorViewportPanel.h"

#include "Subsystem/EditorRendererBridgeSubsystem.h"
#include "Subsystem/EditorSceneBridgeSubsystem.h"
#include "Subsystem/EngineContentSubsystem.h"
#include "Subsystem/SelectionSubsystem.h"

#include "UI/DragDrop/AssetDragDropOperation.h"

namespace Relentless
{
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

	Reply EditorViewportSubsystem::OnCanvasDragOver(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (aDragDropOperation->IsOfType<AssetDragDropOperation>())
			return Reply::Handled();

		return Reply::Unhandled();
	}

	Reply EditorViewportSubsystem::OnDropOnCanvas(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetDragDropOperation>())
			return Reply::Unhandled();

		AssetDragDropOperation& assetDragDropOperation = aDragDropOperation->AsType<AssetDragDropOperation>();
		const std::vector<AssetData>& assetDatas = assetDragDropOperation.GetAssets();
		
		std::vector<AssetData> meshAssetDatas;
		std::vector<AssetData> materialAssetDatas;

		for (const AssetData& assetData : assetDatas)
		{
			if (assetData.Type == Mesh::StaticType())
				meshAssetDatas.push_back(assetData);
			else if (assetData.Type == Material::StaticType())
				materialAssetDatas.push_back(assetData);
		}

		if (meshAssetDatas.empty() && materialAssetDatas.empty())
			return Reply::Unhandled();

		Scene* pActiveScene = m_pEditor->GetActiveScene();
		if (!pActiveScene)
			return Reply::Unhandled();

		EntityManager& entityManager = pActiveScene->GetEntityManager();

		if (!meshAssetDatas.empty())
		{
			EngineContentSubsystem& contentSubsystem = *m_pEditor->GetSubsystem<EngineContentSubsystem>();
			SelectionSubsystem& selectionSubsystem = *m_pEditor->GetSubsystem<SelectionSubsystem>();

			std::vector<entity> newEntities;
			newEntities.reserve(meshAssetDatas.size());

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
					materialHandle = contentSubsystem.GetWhiteMaterialHandle();
				}
				else if (!materialHandle.IsValid())
					materialHandle = contentSubsystem.GetWhiteMaterialHandle();

				const entity newEntity = pActiveScene->CreateEntity(pMesh->GetName().c_str());
				entityManager.Add<MeshFilterComponent>(newEntity).SetMesh(meshHandle);
				entityManager.Add<MeshRendererComponent>(newEntity).SetMaterial(materialHandle);

				newEntities.push_back(newEntity);
			}

			selectionSubsystem.DeselectAllEntities();
			selectionSubsystem.SelectEntities(newEntities);
		}
		
		if (!materialAssetDatas.empty())
		{
			EditorRendererBridgeSubsystem& editorRendererBridge = *m_pEditor->GetSubsystem<EditorRendererBridgeSubsystem>();
			const AssetHandle materialHandle = AssetManager::LoadAsset(materialAssetDatas.back());
			const entity hoveredEntity = editorRendererBridge.GetHoveredEntity();

			if (hoveredEntity != NULL_ENTITY && materialHandle.IsValid())
				entityManager.Get<MeshRendererComponent>(hoveredEntity).SetMaterial(materialHandle);
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
				pEditorViewportPanel->OnCanvasDragOver.Detach(this);
				pEditorViewportPanel->OnCanvasDrop.Detach(this);
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
				pEditorViewportPanel->OnCanvasDragOver.Connect(this, &EditorViewportSubsystem::OnCanvasDragOver);
				pEditorViewportPanel->OnCanvasDrop.Connect(this, &EditorViewportSubsystem::OnDropOnCanvas);
			}

			m_EditorViewports.push_back(pViewportPanel);
		}
	}

	void EditorViewportSubsystem::OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept
	{
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