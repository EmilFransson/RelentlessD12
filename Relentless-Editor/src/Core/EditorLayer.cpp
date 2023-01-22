#include "EditorLayer.h"

namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}, 
		  m_ViewportPanelSize{100.0f, 100.0f}, 
		  m_SceneViewportChanged{ false },
		  m_HoveringSceneViewport{ false },
		  m_ClickedSceneViewPort{ false },
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
			//TODO: Mouse should have the functions stated below, not Window.
			bool isNavigatingScene = m_HoveringSceneViewport;
			if (isNavigatingScene)
			{
				Window::ConfineMouseCursor(vMin.x, vMax.x, vMax.y, vMin.y);
				Window::HideMouseCursor();
				event.StopPropagation();
			}
			break;
		}
		case EventType::RightMouseButtonReleasedEvent:
		{
			bool isNavigatingScene = m_HoveringSceneViewport;
			if (isNavigatingScene)
			{
				Window::FreeMouseCursor();
				Window::ShowMouseCursor();
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
			bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right);
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
		if (entity != NULL_ENTITY)
		{
			if (entity != m_SelectedEntity)
			{
				m_SelectedEntity = entity;
				m_PropertiesPanel.SetSelectedEntity(m_SelectedEntity);
				m_CurrentGizmoType = GizmoType::TRANSLATE;
			}
		}

		if (m_SelectedEntity != NULL_ENTITY && m_CurrentGizmoType != GizmoType::NONE)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

			auto& transformComponent = m_Scene.GetEntityManager().Get<TransformComponent>(m_SelectedEntity);
			DirectX::XMFLOAT4X4 viewMatrix = m_pEditorCamera->GetViewMatrix();
			DirectX::XMFLOAT4X4 projectionMatrix = m_pEditorCamera->GetProjectionMatrix();

			ImGuizmo::Manipulate(*viewMatrix.m, *projectionMatrix.m, (ImGuizmo::OPERATION)m_CurrentGizmoType, ImGuizmo::LOCAL, *transformComponent.Transform.m);
			if (ImGuizmo::IsUsing())
			{
				float translation[3] = { 0 };
				float rotation[3]    = { 0 };
				float scale[3]       = { 0 };

				ImGuizmo::DecomposeMatrixToComponents(*transformComponent.Transform.m, translation, rotation, scale);
				DirectX::XMFLOAT3 rotation2(rotation);
				DirectX::XMVECTOR deltaRot = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&rotation2), DirectX::XMLoadFloat3(&transformComponent.Rotation));

				transformComponent.Translation.x = translation[0];
				transformComponent.Translation.y = translation[1];
				transformComponent.Translation.z = translation[2];

				transformComponent.Scale.x = scale[0];
				transformComponent.Scale.y = scale[1];
				transformComponent.Scale.z = scale[2];

				DirectX::XMVECTOR transformRotationAsVector = DirectX::XMLoadFloat3(&transformComponent.Rotation);
				transformRotationAsVector = DirectX::XMVectorAdd(transformRotationAsVector, deltaRot);
				DirectX::XMStoreFloat3(&transformComponent.Rotation, transformRotationAsVector);

				float angleX = DirectX::XMConvertToRadians(transformComponent.Rotation.x);
				float angleY = DirectX::XMConvertToRadians(transformComponent.Rotation.y);
				float angleZ = DirectX::XMConvertToRadians(transformComponent.Rotation.z);

				DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&transformComponent.Scale)) * DirectX::XMMatrixRotationX(angleX) * DirectX::XMMatrixRotationY(angleY) * DirectX::XMMatrixRotationZ(angleZ) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transformComponent.Translation));
				DirectX::XMStoreFloat4x4(&transformComponent.Transform, world);
			}
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
			ImGui::Text("%s (%d)", m_Scene.GetEntityManager().Get<NameComponent>(m_HoveredEntity).Name.c_str(), m_HoveredEntity);
		}
		else
		{
			ImGui::Text("None");
		}
		ImGui::Text("Selected entity:");
		ImGui::SameLine();
		m_SelectedEntity == NULL_ENTITY ? ImGui::Text("None") : ImGui::Text("%s (%d)", m_Scene.GetEntityManager().Get<NameComponent>(m_SelectedEntity).Name.c_str(), m_SelectedEntity);
		ImGui::SameLine();

		ImGui::End();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_PropertiesPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_MetricsPanel.OnImGuiRender();
	}

	void EditorLayer::OnAttach() noexcept
	{
		m_pEditorCamera = std::move(PerspectiveCamera::Create(DirectX::XMVECTORF32{ 5.0f, 5.0f, -5.0f }, static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y)));

		m_Scene.CreateLight("Directional Light", LightComponent::Type::Directional);
		auto ground = m_Scene.CreateShape<Shape::Cube>();
		auto& tc1 = m_Scene.GetEntityManager().Get<TransformComponent>(ground);
		m_Scene.GetEntityManager().Get<NameComponent>(ground).Name = "Ground";
		m_Scene.GetEntityManager().Get<MeshRendererComponent>(ground).Color = {42.0f / 255.0f, 88.0f / 255.0f, 26.0f / 255.0f };

		tc1.Scale = DirectX::XMFLOAT3{ 6.4f, 0.1f, 6.4f };

		DirectX::XMMATRIX world1 = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&tc1.Scale))
			* DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(tc1.Rotation.x))
			* DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(tc1.Rotation.y))
			* DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(tc1.Rotation.z))
			* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&tc1.Translation));
		DirectX::XMStoreFloat4x4(&tc1.Transform, world1);

		auto cube = m_Scene.CreateShape<Shape::Cube>();
		auto& tc2 = m_Scene.GetEntityManager().Get<TransformComponent>(cube);
		tc2.Translation = {0.0f, 0.55f, 0.0f};

		DirectX::XMMATRIX world2 = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&tc2.Scale))
			* DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(tc2.Rotation.x))
			* DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(tc2.Rotation.y))
			* DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(tc2.Rotation.z))
			* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&tc2.Translation));
		DirectX::XMStoreFloat4x4(&tc2.Transform, world2);

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

		m_PropertiesPanel.SetEntityManager(&m_Scene.GetEntityManager());
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

		const bool resizedSceneViewport = (m_SceneViewportChanged && !Mouse::IsButtonPressed(RLS_BUTTON::Left));
		if (resizedSceneViewport)
			OnSceneViewportChanged();

		if (m_HoveringSceneViewport)
		{
			m_pEditorCamera->Update(deltaTime);
		}

		auto& cb = *m_pEditorCamera->m_pConstantBuffer;
		MemoryManager::Get().UpdateConstantBuffer(cb, &m_pEditorCamera->GetPosition());

		m_Scene.OnUpdate();
	}

	void EditorLayer::OnRender() noexcept
	{
		Renderer3D::Begin(m_pEditorCamera, m_Scene.GetEntityManager());
		m_Scene.GetEntityManager().Bundle<ForwardPassComponent, MeshFilterComponent, MeshRendererComponent>().Do([](entity id, ForwardPassComponent&, MeshFilterComponent&, MeshRendererComponent&)
			{
				Renderer3D::Submit(id);
			});
		Renderer3D::End(m_Scene.GetEntityManager());
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
}
