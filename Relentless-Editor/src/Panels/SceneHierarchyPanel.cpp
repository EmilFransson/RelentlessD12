#include "SceneHierarchyPanel.h"
#include "../Core/EditorLayer.h"
namespace Relentless
{
	SceneHierarchyPanel::SceneHierarchyPanel() noexcept
		: m_pScene{ nullptr },
		  m_SelectedEntity{ NULL_ENTITY }
	{}

	void SceneHierarchyPanel::OnImGuiRender() noexcept
	{
		RLS_ASSERT(m_pScene, "Scene is nullptr.");
		ImGui::Begin("Scene Hierarchy");

		ImGuiTreeNodeFlags sceneNodeflags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;
		sceneNodeflags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
		ImGui::PushFont(boldFont);
		bool opened = ImGui::TreeNodeEx(m_pScene->GetName(), sceneNodeflags, m_pScene->GetName());
		ImGui::PopFont();

		if (opened)
		{
			ImGui::Separator();

			m_pScene->GetEntityManager().Collect<NameComponent>().Do([this](entity e, NameComponent&)
				{
					DrawEntityNode(e);
				});
			ImGui::TreePop();
		}

		if (ImGui::BeginPopupContextWindow(0, 1, false /*over items*/))
		{
			if (ImGui::MenuItem("Create Empty"))
				m_SelectedEntity = m_pScene->CreateEntity("Entity");

			if (ImGui::BeginMenu("Shapes"))
			{
				if (ImGui::MenuItem("Triangle"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Triangle>();
				}
				if (ImGui::MenuItem("Cube"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Cube>();
				}
				if (ImGui::MenuItem("Cylinder"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Cylinder>();
				}
				if (ImGui::MenuItem("Capsule"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Capsule>();
				}
				if (ImGui::MenuItem("Cone"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Cone>();
				}
				if (ImGui::MenuItem("Sphere"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Sphere>();
				}
				if (ImGui::MenuItem("Quad"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Quad>();
				}
				if (ImGui::MenuItem("Plane"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Plane>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Extras"))
			{
				if (ImGui::MenuItem("Utah Teapot"))
				{
					m_SelectedEntity = m_pScene->CreateExtra<Extra::UtahTeapot>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Light"))
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					m_SelectedEntity = m_pScene->CreateLight("Directional Light", LightType::Directional);
				}
				if (ImGui::MenuItem("Point Light"))
				{
					m_SelectedEntity = m_pScene->CreateLight("Point Light", LightType::Point);
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Camera"))
				m_SelectedEntity = m_pScene->CreateCamera("Camera");

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::SetActiveScene(Scene* const pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is nullptr.");
		m_pScene = pScene;
	}

	void SceneHierarchyPanel::DrawEntityNode(const entity entityID) noexcept
	{
		auto& mgr = m_pScene->GetEntityManager();
		ImGuiTreeNodeFlags entityNodeflags = ((entityID == m_SelectedEntity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		entityNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;

		ImGuiStyle* style = &ImGui::GetStyle();
		style->Alpha = (entityID == m_SelectedEntity) ? 1.0f : 0.5f;

		auto& nc = mgr.Get<NameComponent>(entityID);

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entityID, entityNodeflags, nc.Name.c_str());
		if (ImGui::IsItemClicked())
			m_SelectedEntity = entityID;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				m_pScene->DestroyEntity(entityID);
				m_OnEntityDestroyedCallBack(entityID);
				if (m_SelectedEntity == entityID)
				{
					m_SelectedEntity = NULL_ENTITY;
				}
			}

			ImGui::EndPopup();
		}

		if (opened)
			ImGui::TreePop();

		style->Alpha = 1.0f;
	}

	void SceneHierarchyPanel::SetSelectedEntity(const entity entityID) noexcept
	{
		m_SelectedEntity = entityID;
	}

	void SceneHierarchyPanel::SetOnEntityDestroyFunction(std::function<void(entity id)> callBackFunction) noexcept
	{
		m_OnEntityDestroyedCallBack = callBackFunction;
	}
}