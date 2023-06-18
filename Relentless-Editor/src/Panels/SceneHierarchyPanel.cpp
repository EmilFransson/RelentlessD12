#include "SceneHierarchyPanel.h"
#include "../Core/EditorLayer.h"
namespace Relentless
{
	SceneHierarchyPanel::SceneHierarchyPanel() noexcept
		: m_pScene{ nullptr },
		  m_SelectedEntity{ NULL_ENTITY },
		  m_EntityScheduledForDestruction{ NULL_ENTITY }
	{}

	void SceneHierarchyPanel::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

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
					if (m_pScene->GetEntityManager().Has<IsChildComponent>(e))
						return;

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
				if (ImGui::MenuItem("IcoSphere"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::IcoSphere>();
				}
				if (ImGui::MenuItem("Torus"))
				{
					m_SelectedEntity = m_pScene->CreateShape<Shape::Torus>();
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

		//One can drop an entity onto the panel itself to return it to an unparented root state:
		if (ImGui::IsDragDropActive())
		{
			auto panelSize = ImGui::GetWindowSize();
			auto currentCursorPosition = ImGui::GetCursorPos();

			panelSize.x -= 20;
			panelSize.y -= (currentCursorPosition.y + 10);
			ImGui::InvisibleButton("SCENE_HIERARCHY_PANEL_EMPTY_SPACE", panelSize);
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("CHILD_PAYLOAD"))
				{
					auto& mgr = m_pScene->GetEntityManager();

					entity entityToOrphan = *(entity*)payLoad->Data;
					if (mgr.Has<IsChildComponent>(entityToOrphan))
					{
						entity parent = mgr.Get<IsChildComponent>(entityToOrphan).Parent;
						auto& children = mgr.Get<ParentComponent>(parent).Children;
						for (uint32_t childIndex{ 0u }; childIndex < children.size(); ++childIndex)
						{
							if (children[childIndex] == entityToOrphan)
							{
								children.erase(children.begin() + childIndex);
								if (children.empty())
									mgr.Remove<ParentComponent>(parent);

								break;
							}
						}
						mgr.Remove<IsChildComponent>(entityToOrphan);
					}
					mgr.AddOrReplace<RootComponent>(entityToOrphan);
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();

		//Any deletion of an entity in the scene hierarchy has been deferred until now:
		if (m_EntityScheduledForDestruction != NULL_ENTITY)
		{
			//We need to check if a child of the deleted entity is currently selected, and if so, notify the editor layer,
			//as that child will get destroyed in the process.
			if (m_pScene->GetEntityManager().Has<ParentComponent>(m_EntityScheduledForDestruction))
			{
				auto& children = m_pScene->GetEntityManager().Get<ParentComponent>(m_EntityScheduledForDestruction).Children;
				for (auto child : children)
				{
					if (child == m_SelectedEntity)
					{
						m_OnEntityDestroyedCallBack(child);
						m_SelectedEntity = NULL_ENTITY;
						break;
					}
				}
			}

			m_pScene->DestroyEntity(m_EntityScheduledForDestruction);
			m_OnEntityDestroyedCallBack(m_EntityScheduledForDestruction);
			if (m_SelectedEntity == m_EntityScheduledForDestruction)
			{
				m_SelectedEntity = NULL_ENTITY;
			}
			m_EntityScheduledForDestruction = NULL_ENTITY;
		}
	}

	void SceneHierarchyPanel::SetActiveScene(Scene* const pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is nullptr.");
		m_pScene = pScene;
	}

	void SceneHierarchyPanel::DrawEntityNode(const entity entityID) noexcept
	{
		auto& mgr = m_pScene->GetEntityManager();

		ImGuiTreeNodeFlags entityNodeflags = ((entityID == m_SelectedEntity) ? ImGuiTreeNodeFlags_Selected : 0);
		entityNodeflags |= mgr.Has<ParentComponent>(entityID) ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf;
		entityNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;
		entityNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow;

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
				m_EntityScheduledForDestruction = entityID;
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("CHILD_PAYLOAD", &entityID, sizeof(entity), ImGuiCond_::ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("CHILD_PAYLOAD"))
			{
				entity toBecomeChild = *(entity*)payLoad->Data;
				if (entityID != toBecomeChild && !m_pScene->EntityIsDescendant(toBecomeChild, entityID))
				{
					m_pScene->ParentEntity(toBecomeChild, entityID);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (opened && mgr.Exists(entityID) && mgr.Has<ParentComponent>(entityID))
		{
			auto& pc = mgr.Get<ParentComponent>(entityID);
			for (auto child : pc.Children)
				DrawEntityNode(child);
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