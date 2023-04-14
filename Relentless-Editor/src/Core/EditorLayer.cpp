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
				m_pEditorCamera->OnMouseMove();
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

			if (ImGuizmo::IsOver())
				return;

			if (m_HoveredEntity == NULL_ENTITY)
			{
				m_SelectedEntity = NULL_ENTITY;
				m_CurrentGizmoType = GizmoType::NONE;
			}
			else
			{
				m_SelectedEntity = m_HoveredEntity;
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
		auto pUITextureHandle = ImguiLayer::GetUITextureGPUHandle();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Scene", NULL);
		sceneViewPortStartPosition = ImGui::GetCursorStartPos();
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

		ImGui::Image
		(
			(ImTextureID)pUITextureHandle.ptr,
			ImVec2(m_ViewportPanelSize.x, m_ViewportPanelSize.y)
		);

		auto entity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (entity != NULL_ENTITY && entity != m_SelectedEntity)
		{
			m_SelectedEntity = entity;
			m_PropertiesPanel.SetSelectedEntity(m_SelectedEntity);
			m_CurrentGizmoType = GizmoType::TRANSLATE;
		}

		if (m_SelectedEntity != NULL_ENTITY && m_CurrentGizmoType != GizmoType::NONE)
		{
			ManipulateTransformGizmo();
		}

		vMin = ImGui::GetWindowContentRegionMin();
		vMax = ImGui::GetWindowContentRegionMax();
		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		sceneViewPortWindowPosition = ImGui::GetWindowPos();
		m_HoveringSceneViewport = ImGui::IsWindowHovered();

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Stats");
		ImGui::Text("Hovered entity:");
		ImGui::SameLine();

		if (m_HoveredEntity != NULL_ENTITY && m_Scene.GetEntityManager().Exists(m_HoveredEntity))
		{
			ImGui::Text("%s (%d)", m_Scene.GetEntityManager().Get<NameComponent>(m_HoveredEntity).Name.c_str(), (m_HoveredEntity >> 12));
		}
		else
		{
			ImGui::Text("None");
		}
		ImGui::Text("Selected entity:");
		ImGui::SameLine();
		m_SelectedEntity == NULL_ENTITY ? ImGui::Text("None") : ImGui::Text("%s (%d)", m_Scene.GetEntityManager().Get<NameComponent>(m_SelectedEntity).Name.c_str(), (m_SelectedEntity >> 12));
		ImGui::SameLine();

		ImGui::End();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_PropertiesPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_MetricsPanel.OnImGuiRender();
	}

	struct DATA
	{
		DATA(const std::string& aa, double bb, double cc, double dd)
			: a{aa}, b{bb}, c{cc}, d{dd}
		{
		}

		std::string a;
		double b;
		double c;
		double d;
	};

	void EditorLayer::OnAttach() noexcept
	{
		LoadStarterMeshes();
		m_pEditorCamera = std::move(PerspectiveCamera::Create(DirectX::XMVECTORF32{ 5.0f, 5.0f, -5.0f }, static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y)));
		m_Scene.CreateLight("Directional Light", LightType::Directional);
		{
			auto ground = m_Scene.CreateShape<Shape::Cube>();
			m_Scene.GetEntityManager().Get<NameComponent>(ground).Name = "Ground";
			m_Scene.GetEntityManager().Get<MeshRendererComponent>(ground).Color = { 42.0f / 255.0f, 88.0f / 255.0f, 26.0f / 255.0f };

			auto& tc = m_Scene.GetEntityManager().Get<TransformComponent>(ground);
			tc.Scale = DirectX::XMFLOAT3{ 6.4f, 0.1f, 6.4f };
		}

		{
			auto cube = m_Scene.CreateShape<Shape::Quad>();
			auto& tc = m_Scene.GetEntityManager().Get<TransformComponent>(cube);
			tc.Translation = { 0.0f, 0.55f, 0.0f };
		}

		m_SceneHierarchyPanel.SetActiveScene(&m_Scene);
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

		m_PropertiesPanel.SetActiveScene(&m_Scene);
		MemoryManager::Get().GetUploadBuffer()->Upload();
		m_Scene.SetViewportPanelSize(m_ViewportPanelSize);
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		if (!m_SceneViewportChanged)
		{
			if (m_HoveringSceneViewport)
			{
				auto& clientRect = Window::GetClientRect();
				auto [x, y] = Mouse::GetCoordinates();

				y -= static_cast<uint32_t>(sceneViewPortStartPosition.y);
				x -= static_cast<uint32_t>(sceneViewPortStartPosition.x);

				x -= static_cast<uint32_t>((sceneViewPortWindowPosition.x - clientRect.left));
				y -= static_cast<uint32_t>((sceneViewPortWindowPosition.y - clientRect.top));

				m_HoveredEntity = Renderer3D::GetHoveredEntity(x, y);
			}
			else
				m_HoveredEntity = NULL_ENTITY;
		}

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
			m_pEditorCamera->Update(deltaTime);
		}

		auto& cb = *m_pEditorCamera->m_pConstantBuffer;
		MemoryManager::Get().UpdateConstantBuffer(cb, &m_pEditorCamera->GetPosition());

		m_Scene.OnUpdate();
	}

	void EditorLayer::OnRender() noexcept
	{
		Renderer3D::Begin(m_pEditorCamera, m_Scene);
		m_Scene.GetEntityManager().Collect<ForwardPassComponent, MeshFilterComponent, MeshRendererComponent>().Do([](entity id)
			{
				Renderer3D::Submit(id);
			});
		Renderer3D::End(m_Scene.GetEntityManager());
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
		std::for_each(std::execution::par, starterMeshes.begin(), starterMeshes.end(), [&](std::string& starterMesh)
			{
				std::string fullMeshPath(meshPath + std::string(starterMesh));
				std::string fileName = starterMesh.substr(0, starterMesh.find_first_of("."));
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

	void EditorLayer::OnSceneViewportChanged() noexcept
	{
		m_ViewportPanelSize.x = std::max(1.0f, m_ViewportPanelSize.x);
		m_ViewportPanelSize.y = std::max(1.0f, m_ViewportPanelSize.y);
		m_Scene.SetViewportPanelSize(m_ViewportPanelSize);

		m_pEditorCamera->RecalculateProjectionMatrix(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		Renderer3D::OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		ImguiLayer::OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_SceneViewportChanged = false;
	}

	void EditorLayer::DestroySelectedEntity() noexcept
	{
		if (m_SelectedEntity == NULL_ENTITY)
			return;

		m_Scene.DestroyEntity(m_SelectedEntity);
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

		auto& mgr = m_Scene.GetEntityManager();
		auto newEntity = m_Scene.CreateEntityWithUUID(mgr.Get<NameComponent>(m_SelectedEntity).Name.c_str());
		auto& tc1 = mgr.Get<TransformComponent>(m_SelectedEntity);
		auto& tc2 = mgr.Get<TransformComponent>(newEntity);
		tc2.Translation = tc1.Translation;
		tc2.Rotation = tc1.Rotation;
		tc2.Scale = tc1.Scale;
		tc2.Transform = tc1.Transform;

		if (mgr.Has<MeshFilterComponent>(m_SelectedEntity))
		{
			auto& mfc = mgr.Get<MeshFilterComponent>(m_SelectedEntity);
			auto& mfcNew = mgr.Add<MeshFilterComponent>(newEntity);

			mfcNew.VertexBufferID = mfc.VertexBufferID;
			mfcNew.IndexBufferID = mfc.IndexBufferID;
		}
		if (mgr.Has<MeshRendererComponent>(m_SelectedEntity))
		{
			mgr.Add<DirtyMeshRendererComponent>(newEntity);
			auto& mrcNew = mgr.Add<MeshRendererComponent>(newEntity);
			mrcNew.constantBufferID = MemoryManager::Get().CreateConstantBuffer(sizeof(MeshRendererComponent) - sizeof(uint32_t));
			auto& mrc = mgr.Get<MeshRendererComponent>(m_SelectedEntity);
			mrcNew.Color = mrc.Color;
		}
		if (mgr.Has<ForwardPassComponent>(m_SelectedEntity))
			mgr.Add<ForwardPassComponent>(newEntity);
		if (mgr.Has<DirectionalLightComponent>(m_SelectedEntity))
		{
			auto& newDlc = mgr.Add<DirectionalLightComponent>(newEntity);
			auto& dlc = mgr.Get<DirectionalLightComponent>(m_SelectedEntity);
			m_Scene.GetLightManager().AllocateDirectionalLight(newEntity);
			newDlc.Color = dlc.Color;
			newDlc.Intensity = dlc.Intensity;
		}
		else if (mgr.Has<PointLightComponent>(m_SelectedEntity))
		{
			auto& newPlc = mgr.Add<PointLightComponent>(newEntity);
			auto& plc = mgr.Get<PointLightComponent>(m_SelectedEntity);
			m_Scene.GetLightManager().AllocatePointLight(newEntity);
			newPlc.Color = plc.Color;
			newPlc.Intensity = plc.Intensity;
		}

		m_SelectedEntity = newEntity;
		m_SceneHierarchyPanel.SetSelectedEntity(m_SelectedEntity);
		m_PropertiesPanel.SetSelectedEntity(m_SelectedEntity);
	}

	void EditorLayer::ManipulateTransformGizmo() noexcept
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		auto& transformComponent = m_Scene.GetEntityManager().Get<TransformComponent>(m_SelectedEntity);

		static DirectX::XMFLOAT3 snapTranslationScale{ 1.0f, 1.0f, 1.0f };
		static float snapRotation{ 10.0f };

		DirectX::XMFLOAT4X4 visualizationMatrix = m_Scene.GetEntityManager().Get<TransformComponent>(m_SelectedEntity).Transform;
		float translationBefore[3]	= { 0.0f };
		float rotationBefore[3]		= { 0.0f };
		float scaleBefore[3]		= { 0.0f };

		ImGuizmo::DecomposeMatrixToComponents(*visualizationMatrix.m, translationBefore, rotationBefore, scaleBefore);

		ImGuizmo::MODE mode;
		if (!m_Scene.GetEntityManager().Has<IsChildComponent>(m_SelectedEntity))
			mode = ImGuizmo::WORLD;
		else
			mode = (ImGuizmo::OPERATION)m_CurrentGizmoType == ImGuizmo::OPERATION::SCALE ? ImGuizmo::LOCAL : ImGuizmo::WORLD;

		if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
			ImGuizmo::Manipulate(*m_pEditorCamera->GetViewMatrix().m, *m_pEditorCamera->GetProjectionMatrix().m, (ImGuizmo::OPERATION)m_CurrentGizmoType, mode, *visualizationMatrix.m, nullptr, (m_CurrentGizmoType == GizmoType::ROTATE) ? &snapRotation : &snapTranslationScale.x);
		else
		{
			ImGuizmo::Manipulate(*m_pEditorCamera->GetViewMatrix().m, *m_pEditorCamera->GetProjectionMatrix().m, (ImGuizmo::OPERATION)m_CurrentGizmoType, mode, *visualizationMatrix.m);
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

			if (m_Scene.GetEntityManager().Has<IsChildComponent>(m_SelectedEntity))
			{
				auto& parentScale = m_Scene.GetEntityManager().Get<TransformComponent>(m_Scene.GetEntityManager().Get<IsChildComponent>(m_SelectedEntity).Parent).Scale;
				
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

			m_Scene.GetEntityManager().AddOrReplace<DirtyTransformComponent>(m_SelectedEntity).AdjustedWorldSpace = true;
		}
	}
}
