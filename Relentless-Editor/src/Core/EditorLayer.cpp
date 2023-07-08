#include "EditorLayer.h"

namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}, 
		  m_ViewportPanelSize{100.0f, 100.0f}, 
		  m_SceneViewportChanged{ false },
		  m_HoveringSceneViewport{ false },
		  m_HoveredEntity{ NULL_ENTITY },
		  m_SelectedEntity{ NULL_ENTITY },
		  m_CurrentGizmoType{ GizmoType::NONE }
	{}

	void EditorLayer::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::RawMouseMoveEvent:
		{
			const bool isNavigatingScene = (m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right));
			if (isNavigatingScene)
				m_pScene->GetEditorCamera()->OnMouseMove();
			event.StopPropagation();
			break;
		}
		case EventType::RightMouseButtonPressedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport;
			if (isNavigatingScene)
			{
				Mouse::ConfineCursor(vMin.x, vMax.x, vMax.y, vMin.y);
				Mouse::HideCursor();
				event.StopPropagation();
			}
			break;
		}
		case EventType::RightMouseButtonReleasedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport;
			if (isNavigatingScene)
			{
				Mouse::FreeCursor();
				Mouse::ShowCursor();
				event.StopPropagation();
			}
			break;
		}
		case EventType::LeftMouseButtonReleasedEvent:
		{
			if (!m_HoveringSceneViewport)
				return;

			if (ImGuizmo::IsUsing())
				return;

			if (m_HoveredEntity == NULL_ENTITY)
			{
				if (m_SelectedEntity != NULL_ENTITY)
				{
					m_pScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);
				}
				m_SelectedEntity = NULL_ENTITY;
				m_CurrentGizmoType = GizmoType::NONE;
			}
			else
			{
				if (m_SelectedEntity != NULL_ENTITY)
					m_pScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);

				m_SelectedEntity = m_HoveredEntity;
				m_pScene->GetEntityManager().Add<SelectedInEditorComponent>(m_SelectedEntity);
				m_CurrentGizmoType = GizmoType::TRANSLATE;
			}
			m_SceneHierarchyPanel.SetSelectedEntity(m_SelectedEntity);
			m_PropertiesPanel.SetSelectedEntity(m_SelectedEntity);

			break;
		}
		case EventType::KeyPressedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right);
			if (!isNavigatingScene)
			{
				RLS_KEY key = EVENT(KeyPressedEvent).key;
				if (key == RLS_KEY::Q)
					m_CurrentGizmoType = GizmoType::NONE;
				else if (key == RLS_KEY::W)
					m_CurrentGizmoType = (GizmoType)ImGuizmo::TRANSLATE;
				else if (key == RLS_KEY::E)
					m_CurrentGizmoType = (GizmoType)ImGuizmo::ROTATE;
				else if (key == RLS_KEY::R)
					m_CurrentGizmoType = (GizmoType)ImGuizmo::SCALE;
				else if (key == RLS_KEY::D && Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
					CopySelectedEntity();
				else if (key == RLS_KEY::Delete)
					DestroySelectedEntity();
			}
			event.StopPropagation();
			break;
		}
		}
	}

	void EditorLayer::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Scene", NULL);

		vMin = ImGui::GetWindowContentRegionMin();
		vMax = ImGui::GetWindowContentRegionMax();
		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		m_HoveringSceneViewport = ImGui::IsWindowHovered();

		constexpr float toolBarPadding = 24.0f;
		ImVec2 mousePosition = ImGui::GetMousePos();
		ImVec2 windowPosition = ImGui::GetWindowPos();
		ImVec2 mousePositionInSceneClientArea = ImVec2(mousePosition.x - windowPosition.x, mousePosition.y - windowPosition.y - toolBarPadding);
		m_pScene->SetMousePosition(mousePositionInSceneClientArea);

		if (m_ViewportPanelSize.x != ImGui::GetContentRegionAvail().x || m_ViewportPanelSize.y != ImGui::GetContentRegionAvail().y)
		{
			//Verify values are valid (as is not the case when shutting down the program!)
			if (!(ImGui::GetContentRegionAvail().x < 0.0f) && !(ImGui::GetContentRegionAvail().y < 0.0f))
			{
				m_ViewportPanelSize.x = ImGui::GetContentRegionAvail().x;
				m_ViewportPanelSize.y = ImGui::GetContentRegionAvail().y;
				m_SceneViewportChanged = true;
			}
		}

		auto UITextureHandle = MasterRenderer::GetFrameBuffer()->GetOutput(0)->GetSRVDescriptorHandle().GPUHandle;
		ImGui::Image
		(
			(ImTextureID)UITextureHandle.ptr,
			ImVec2(m_ViewportPanelSize.x, m_ViewportPanelSize.y)
		);

		if (m_SelectedEntity != NULL_ENTITY && m_CurrentGizmoType != GizmoType::NONE)
		{
			ManipulateTransformGizmo();
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Stats");
		ImGui::Text("Hovered entity:");
		ImGui::SameLine();

		

		if (m_HoveredEntity != NULL_ENTITY && m_pScene->GetEntityManager().Exists(m_HoveredEntity))
		{
			ImGui::Text("%s (%d)", m_pScene->GetEntityManager().Get<NameComponent>(m_HoveredEntity).Name.c_str(), (m_HoveredEntity >> 12));
		}
		else
		{
			ImGui::Text("None");
		}
		ImGui::Text("Selected entity:");
		ImGui::SameLine();
		m_SelectedEntity == NULL_ENTITY ? ImGui::Text("None") : ImGui::Text("%s (%d)", m_pScene->GetEntityManager().Get<NameComponent>(m_SelectedEntity).Name.c_str(), (m_SelectedEntity >> 12));
		ImGui::SameLine();

		ImGui::End();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_SceneRendererPanel.OnImGuiRender();
		m_PropertiesPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_MetricsPanel.OnImGuiRender();
	}

	void EditorLayer::OnAttach() noexcept
	{
		LoadStarterMeshes();
		CreateStartScene();
		m_pSceneRenderer = std::make_shared<SceneRenderer>(m_pScene);

		m_PropertiesPanel.SetActiveScene(m_pScene.get());
		m_SceneHierarchyPanel.SetActiveScene(m_pScene.get());
		m_SceneRendererPanel.SetActiveRenderer(m_pSceneRenderer);

		//TODO: Collapse into one switch-case-callback-function
		m_SceneHierarchyPanel.SetOnEntityDestroyFunction([this](entity entityID)
			{
				if (entityID == m_SelectedEntity)
				{
					m_PropertiesPanel.SetSelectedEntity(NULL_ENTITY);
					m_SelectedEntity = NULL_ENTITY;
				}
				if (entityID == m_HoveredEntity)
					m_HoveredEntity = NULL_ENTITY;
			});
		m_SceneHierarchyPanel.SetOnEntityCreatedFunction([this](entity entityID)
			{
				if (m_SelectedEntity != NULL_ENTITY)
				{
					m_pScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);
				}
				m_pScene->GetEntityManager().Add<SelectedInEditorComponent>(entityID);
				m_PropertiesPanel.SetSelectedEntity(entityID);
				m_SelectedEntity = entityID;

				if (m_CurrentGizmoType == GizmoType::NONE)
				{
					m_CurrentGizmoType = GizmoType::TRANSLATE;
				}
			});
		m_SceneHierarchyPanel.SetOnEntitySelectedFunction([this](entity entityID)
			{
				if (m_SelectedEntity != NULL_ENTITY)
				{
					m_pScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);
				}
				m_pScene->GetEntityManager().Add<SelectedInEditorComponent>(entityID);
				m_PropertiesPanel.SetSelectedEntity(entityID);
				m_SelectedEntity = entityID;

				if (m_CurrentGizmoType == GizmoType::NONE)
				{
					m_CurrentGizmoType = GizmoType::TRANSLATE;
				}
			});

		MemoryManager::Get().GetUploadBuffer()->Upload();
		m_pScene->SetViewportPanelSize(m_ViewportPanelSize);
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		PROFILE_FUNC;

		if (m_SceneViewportChanged)
			OnSceneViewportChanged();

		//CameraController should handle all this (and more for an even better editor camera experience!)
		const bool isNavigatingViewport = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right);
		if (isNavigatingViewport && 
			(		Keyboard::IsKeyPressed(RLS_KEY::W) 
				||	Keyboard::IsKeyPressed(RLS_KEY::S) 
				||	Keyboard::IsKeyPressed(RLS_KEY::A) 
				||	Keyboard::IsKeyPressed(RLS_KEY::D)
				||  Keyboard::IsKeyPressed(RLS_KEY::Q)
				||  Keyboard::IsKeyPressed(RLS_KEY::E)))
		{
			m_pScene->GetEditorCamera()->Update(deltaTime);
		}

		auto& cb = *m_pScene->GetEditorCamera()->m_pConstantBuffer;
		MemoryManager::Get().UpdateConstantBuffer(cb, &m_pScene->GetEditorCamera()->GetPosition());

		m_pScene->OnUpdate();
	}

	void EditorLayer::OnRender() noexcept
	{
		PROFILE_FUNC;

		m_pSceneRenderer->Begin();
		m_pSceneRenderer->IssueRenderPasses();
		m_pSceneRenderer->End();
	}

	//NOT CURRENTLY CORRECT ACTUALLY:
	//YES THE GPU HAS STARTED HOWEVER WE DO NOT KNOW WHETHER IT IS FINISHED AT ALL 
	//WITH RENDERING THIS FRAME (SEE Application.cpp for more details)
	void EditorLayer::OnPostRender() noexcept
	{
		if (m_HoveringSceneViewport)
		{
			m_HoveredEntity = m_pSceneRenderer->GetHoveredEntity();
		}
		else
			m_HoveredEntity = NULL_ENTITY;

		m_SceneRendererPanel.OnPostRender();
	}

	void EditorLayer::LoadStarterMeshes() noexcept
	{
		auto& assetManager = AssetManager::Get();
		std::vector<std::string> starterMeshes
		{
			"Capsule.gltf",
			"Cone.gltf",
			"Cylinder.gltf",
			"IcoSphere.obj",
			"Plane.gltf",
			"Quad.gltf",
			"Sphere.obj",
			"Torus.obj",
			"Triangle.gltf",
			"UtahTeapot.gltf"
		};


		std::string meshPath = std::string(ENGINE_ASSET_DIRECTORY) + std::string("Meshes/");
		std::for_each(std::execution::par, starterMeshes.begin(), starterMeshes.end(), [&](std::string& starterMeshName)
			{
				std::string fullMeshPath(meshPath + std::string(starterMeshName));
				std::string fileName = starterMeshName.substr(0, starterMeshName.find_first_of("."));
				if (!assetManager.HasLoaded(fileName + " Vertex Buffer"))
				{
					MeshFactory meshFactory;
					auto& starterMesh = meshFactory.LoadFromFile(fullMeshPath)[0];

					VertexBuffer::Specification vbSpec
					{
						.NrOfVertices = (uint32_t)starterMesh.Vertices.size(),
						.TotalSizeInBytes = (uint32_t)starterMesh.Vertices.size() * sizeof(SimpleVertex),
						.Stride = sizeof(SimpleVertex),
						.pBuffer = (void*)starterMesh.Vertices.data(),
						.Name = fileName + std::string(" Vertex Buffer")
					};

					IndexBuffer::Specification ibSpec
					{
						.NrOfIndices = (uint32_t)starterMesh.Indices.size(),
						.TotalSizeInBytes = (uint32_t)starterMesh.Indices.size() * sizeof(uint32_t),
						.Stride = sizeof(uint32_t),
						.pBuffer = (void*)starterMesh.Indices.data(),
						.Name = fileName + std::string(" Index Buffer")
					};
					AssetManager::Get().Load<VertexBuffer>(vbSpec.Name, &vbSpec);
					AssetManager::Get().Load<IndexBuffer>(ibSpec.Name, &ibSpec);
				}
			});
	}

	void EditorLayer::CreateStartScene() noexcept
	{
		m_pScene = std::make_shared<Scene>();

		m_pScene->CreateLight("Directional Light", LightType::Directional);
		
		{
			auto ground = m_pScene->CreateShape<Shape::Cube>();
			m_pScene->GetEntityManager().Get<NameComponent>(ground).Name = "Ground";
			Material& material = AssetManager::Get().Get<Material>(m_pScene->GetEntityManager().Get<MeshRendererComponent>(ground).MaterialHandle);
			material.m_AlbedoColor = { 42.0f / 255.0f, 88.0f / 255.0f, 26.0f / 255.0f };

			auto& tc = m_pScene->GetEntityManager().Get<TransformComponent>(ground);
			tc.Scale = DirectX::XMFLOAT3{ 6.4f, 0.1f, 6.4f };
		}

		{
			auto cube = m_pScene->CreateShape<Shape::Cube>();
			auto& tc = m_pScene->GetEntityManager().Get<TransformComponent>(cube);
			tc.Translation = { 0.0f, 0.55f, 0.0f };
		}
	}

	void EditorLayer::OnSceneViewportChanged() noexcept
	{
		m_ViewportPanelSize.x = std::max(1.0f, m_ViewportPanelSize.x);
		m_ViewportPanelSize.y = std::max(1.0f, m_ViewportPanelSize.y);
		m_pScene->SetViewportPanelSize(m_ViewportPanelSize);

		m_pScene->GetEditorCamera()->RecalculateProjectionMatrix(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_pSceneRenderer->OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_SceneViewportChanged = false;
	}

	void EditorLayer::DestroySelectedEntity() noexcept
	{
		if (m_SelectedEntity == NULL_ENTITY)
			return;

		m_pScene->DestroyEntity(m_SelectedEntity);
		m_SceneHierarchyPanel.SetSelectedEntity(NULL_ENTITY);
		m_PropertiesPanel.SetSelectedEntity(NULL_ENTITY);
		m_SelectedEntity = NULL_ENTITY;
	}

	//Should be extended to perform full copies, however as for now
	//it serves as an easy way of getting more shapes on screen.
	void EditorLayer::CopySelectedEntity() noexcept
	{
		if (m_SelectedEntity == NULL_ENTITY)
			return;

		auto& mgr = m_pScene->GetEntityManager();
		auto newEntity = m_pScene->CreateEntityWithUUID(mgr.Get<NameComponent>(m_SelectedEntity).Name.c_str());
		auto& tc1 = mgr.Get<TransformComponent>(m_SelectedEntity);
		auto& tc2 = mgr.Get<TransformComponent>(newEntity);
		tc2.Translation = tc1.Translation;
		tc2.Rotation = tc1.Rotation;
		tc2.Scale = tc1.Scale;
		tc2.Transform = tc1.Transform;

		if (mgr.Has<MeshFilterComponent>(m_SelectedEntity))
		{
			auto& mfcNew = mgr.Add<MeshFilterComponent>(newEntity);
			auto& mfc = mgr.Get<MeshFilterComponent>(m_SelectedEntity);

			mfcNew.VertexBufferID = mfc.VertexBufferID;
			mfcNew.IndexBufferID = mfc.IndexBufferID;
		}
		if (mgr.Has<MeshRendererComponent>(m_SelectedEntity))
		{
			auto& mrcNew = mgr.Add<MeshRendererComponent>(newEntity);
			auto& mrc = mgr.Get<MeshRendererComponent>(m_SelectedEntity);
			mrcNew.MaterialHandle = mrc.MaterialHandle;
			mgr.Add<DirtyMeshRendererComponent>(newEntity);
		}
		if (mgr.Has<ForwardPassComponent>(m_SelectedEntity))
			mgr.Add<ForwardPassComponent>(newEntity);
		if (mgr.Has<DirectionalLightComponent>(m_SelectedEntity))
		{
			auto& newDlc = mgr.Add<DirectionalLightComponent>(newEntity);
			auto& dlc = mgr.Get<DirectionalLightComponent>(m_SelectedEntity);
			m_pScene->GetLightManager().AllocateDirectionalLight(newEntity);
			newDlc.Color = dlc.Color;
			newDlc.Intensity = dlc.Intensity;
		}
		else if (mgr.Has<PointLightComponent>(m_SelectedEntity))
		{
			auto& newPlc = mgr.Add<PointLightComponent>(newEntity);
			auto& plc = mgr.Get<PointLightComponent>(m_SelectedEntity);
			m_pScene->GetLightManager().AllocatePointLight(newEntity);
			newPlc.Color = plc.Color;
			newPlc.Intensity = plc.Intensity;
		}

		mgr.Remove<SelectedInEditorComponent>(m_SelectedEntity);
		mgr.AddOrReplace<SelectedInEditorComponent>(newEntity);

		m_SelectedEntity = newEntity;
		m_SceneHierarchyPanel.SetSelectedEntity(m_SelectedEntity);
		m_PropertiesPanel.SetSelectedEntity(m_SelectedEntity);
	}

	void EditorLayer::ManipulateTransformGizmo() noexcept
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		auto& transformComponent = m_pScene->GetEntityManager().Get<TransformComponent>(m_SelectedEntity);

		static DirectX::XMFLOAT3 snapTranslationScale{ 1.0f, 1.0f, 1.0f };
		static float snapRotation{ 10.0f };

		DirectX::XMFLOAT4X4 visualizationMatrix = m_pScene->GetEntityManager().Get<TransformComponent>(m_SelectedEntity).Transform;
		float translationBefore[3]	= { 0.0f };
		float rotationBefore[3]		= { 0.0f };
		float scaleBefore[3]		= { 0.0f };

		ImGuizmo::DecomposeMatrixToComponents(*visualizationMatrix.m, translationBefore, rotationBefore, scaleBefore);

		ImGuizmo::MODE mode;
		if (!m_pScene->GetEntityManager().Has<IsChildComponent>(m_SelectedEntity))
			mode = ImGuizmo::WORLD;
		else
			mode = (ImGuizmo::OPERATION)m_CurrentGizmoType == ImGuizmo::OPERATION::SCALE ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

		if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
			ImGuizmo::Manipulate(*m_pScene->GetEditorCamera()->GetViewMatrix().m, *m_pScene->GetEditorCamera()->GetProjectionMatrix().m, (ImGuizmo::OPERATION)m_CurrentGizmoType, mode, *visualizationMatrix.m, nullptr, (m_CurrentGizmoType == GizmoType::ROTATE) ? &snapRotation : &snapTranslationScale.x);
		else
		{
			ImGuizmo::Manipulate(*m_pScene->GetEditorCamera()->GetViewMatrix().m, *m_pScene->GetEditorCamera()->GetProjectionMatrix().m, (ImGuizmo::OPERATION)m_CurrentGizmoType, mode, *visualizationMatrix.m);
		}
		if (ImGuizmo::IsUsing())
		{
			float translationAfter[3]	= { 0.0f };
			float rotationAfter[3]		= { 0.0f };
			float scaleAfter[3]			= { 0.0f };

			ImGuizmo::DecomposeMatrixToComponents(*visualizationMatrix.m, translationAfter, rotationAfter, scaleAfter);
		
			float translationOffsetX = translationAfter[0] - translationBefore[0];
			float translationOffsetY = translationAfter[1] - translationBefore[1];
			float translationOffsetZ = translationAfter[2] - translationBefore[2];

			float rotationOffsetX = rotationAfter[0] - rotationBefore[0];
			float rotationOffsetY = rotationAfter[1] - rotationBefore[1];
			float rotationOffsetZ = rotationAfter[2] - rotationBefore[2];

			float scaleOffsetX = scaleAfter[0] - scaleBefore[0];
			float scaleOffsetY = scaleAfter[1] - scaleBefore[1];
			float scaleOffsetZ = scaleAfter[2] - scaleBefore[2];

			if (m_pScene->GetEntityManager().Has<IsChildComponent>(m_SelectedEntity))
			{
				auto& parentScale = m_pScene->GetEntityManager().Get<TransformComponent>(m_pScene->GetEntityManager().Get<IsChildComponent>(m_SelectedEntity).Parent).Scale;
				
				translationOffsetX /= parentScale.x;
				translationOffsetY /= parentScale.y;
				translationOffsetZ /= parentScale.z;

				rotationOffsetX /= parentScale.x;
				rotationOffsetY /= parentScale.y;
				rotationOffsetZ /= parentScale.z;

				scaleOffsetX /= parentScale.x;
				scaleOffsetY /= parentScale.y;
				scaleOffsetZ /= parentScale.z;

			}

			transformComponent.Translation.x += translationOffsetX;
			transformComponent.Translation.y += translationOffsetY;
			transformComponent.Translation.z += translationOffsetZ;

			transformComponent.Scale.x += std::clamp(scaleOffsetX, -0.5f, 0.5f);
			transformComponent.Scale.y += std::clamp(scaleOffsetY, -0.5f, 0.5f);
			transformComponent.Scale.z += std::clamp(scaleOffsetZ, -0.5f, 0.5f);

			transformComponent.Rotation.x += rotationOffsetX;
			transformComponent.Rotation.y += rotationOffsetY;
			transformComponent.Rotation.z += rotationOffsetZ;

			m_pScene->GetEntityManager().AddOrReplace<DirtyTransformComponent>(m_SelectedEntity).AdjustedWorldSpace = true;
		}
	}
}
