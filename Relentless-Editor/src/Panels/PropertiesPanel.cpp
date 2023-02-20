#include "PropertiesPanel.h"
namespace Relentless
{
	PropertiesPanel::PropertiesPanel() noexcept
		: m_SelectedEntity{ NULL_ENTITY },
		  m_pScene{ nullptr }
	{}

	void PropertiesPanel::OnImGuiRender() noexcept
	{
		ImGui::Begin("Properties");
		if (m_SelectedEntity != NULL_ENTITY)
		{
			DrawAllComponentNodes();
		}
		ImGui::End();
	}

	void PropertiesPanel::SetSelectedEntity(const entity entityID) noexcept
	{
		m_SelectedEntity = entityID;
		m_FormattingName = false;
	}

	void PropertiesPanel::SetActiveScene(Scene* const pScene) noexcept
	{
		RLS_ASSERT(pScene, "Entity manager is nullptr.");
		m_pScene = pScene;
	}

	void PropertiesPanel::DrawAllComponentNodes()
	{
		auto& nc = m_pScene->GetEntityManager().Get<NameComponent>(m_SelectedEntity);

		if (m_FormattingName == false)
		{
			char buff[33];
			buff[0] = buff[1] = '#';
			char* bPtr = buff;
			bPtr += 2;
#pragma warning(push, 0)
			_itoa(m_SelectedEntity, bPtr, 2);
#pragma warning(pop)			
			bool shouldRender = m_pScene->GetEntityManager().Has<ForwardPassComponent>(m_SelectedEntity);
			if (ImGui::Checkbox(buff, &shouldRender))
			{
				if (!shouldRender)
					m_pScene->GetEntityManager().Remove<ForwardPassComponent>(m_SelectedEntity);
				else
					m_pScene->GetEntityManager().Add<ForwardPassComponent>(m_SelectedEntity);
			}

			ImGuiIO& io = ImGui::GetIO();
			auto font = io.Fonts->Fonts[1];

			ImGui::PushFont(font);
			ImVec2 textSize = ImGui::CalcTextSize(nc.Name.c_str());
			ImGui::SameLine((ImGui::GetWindowWidth() / 2.0f) - (textSize.x / 2.0f));
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (ImGui::GetFontSize() / 6.0f));
			ImGui::Text(nc.Name.c_str());
			ImGui::PopFont();

			if (ImGui::IsItemClicked())
				m_FormattingName = true;
		}
		else
		{
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy_s(buffer, sizeof(buffer), nc.Name.c_str());
			ImGui::Text("Name");
			ImGui::SameLine();
			if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
				nc.Name = std::string(buffer);

			if (ImGui::IsItemDeactivated())
				m_FormattingName = false;

			if (Mouse::IsButtonPressed(RLS_BUTTON::Left))
			{
				if (!ImGui::IsItemHovered())
					m_FormattingName = false;
			}
		}
		ImGui::Separator();
		
		constexpr const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		const bool opened = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), flags, "Transform");
		if (opened)
		{
			auto& tc = m_pScene->GetEntityManager().Get<TransformComponent>(m_SelectedEntity);
			bool changedValues = DrawVec3Control("Position", tc.Translation, 0.06f);
			ImGui::Separator();
			changedValues |= DrawVec3Control("Rotation", tc.Rotation, 0.03f);
			ImGui::Separator();
			changedValues |= DrawVec3Control("Scale", tc.Scale, 0.03f, 1.0f, 0.01f);
			
			if (changedValues)
			{
				DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&tc.Scale))
					* DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(tc.Rotation.x))
					* DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(tc.Rotation.y))
					* DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(tc.Rotation.z))
					* DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&tc.Translation));
				DirectX::XMStoreFloat4x4(&tc.Transform, world);

				//Temporary for now!
				if (m_pScene->GetEntityManager().HasAnyOf<DirectionalLightComponent, PointLightComponent>(m_SelectedEntity))
				{
					m_pScene->GetEntityManager().AddOrReplace<DirtyLightComponent>(m_SelectedEntity);
				}
			}
			
			ImGui::TreePop();
		}

		DrawComponentNode<DirectionalLightComponent>("Light", [this]()
			{
				constexpr const char* lightTypeStrings[] = { "Directional", "Point" };
				constexpr const char* currentLightTypeString = lightTypeStrings[(int)LightType::Directional];
				auto& lc = m_pScene->GetEntityManager().Get<DirectionalLightComponent>(m_SelectedEntity);
				
				if (ImGui::BeginCombo("Type", currentLightTypeString))
				{
					for (uint8_t i = 0u; i < ARRAYSIZE(lightTypeStrings); i++)
					{
						bool isSelected = currentLightTypeString == lightTypeStrings[i];
						if (ImGui::Selectable(lightTypeStrings[i], isSelected))
						{
							if (!isSelected)
							{
								auto color = lc.Color;
								auto intensity = lc.Intensity;
								m_pScene->GetEntityManager().Remove<DirectionalLightComponent>(m_SelectedEntity);
								m_pScene->GetLightManager().DeallocateDirectionalLight(m_SelectedEntity);
								auto& dlc = m_pScene->GetEntityManager().Add<PointLightComponent>(m_SelectedEntity);
								m_pScene->GetLightManager().AllocatePointLight(m_SelectedEntity);
								dlc.Color = color;
								dlc.Intensity = intensity;
								m_pScene->GetEntityManager().AddOrReplace<DirtyLightComponent>(m_SelectedEntity);
							}
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (ImGui::ColorEdit3("Color", &lc.Color.x))
					m_pScene->GetEntityManager().AddOrReplace<DirtyLightComponent>(m_SelectedEntity);

				if (ImGui::DragFloat("Intensity", &lc.Intensity, 0.05f, 0.0f, 1.0f))
					m_pScene->GetEntityManager().AddOrReplace<DirtyLightComponent>(m_SelectedEntity);
			});

		DrawComponentNode<PointLightComponent>("Light", [&]()
			{
				constexpr const char* lightTypeStrings[] = { "Directional", "Point" };
				constexpr const char* currentLightTypeString = lightTypeStrings[(int)LightType::Point];
				auto& lc = m_pScene->GetEntityManager().Get<PointLightComponent>(m_SelectedEntity);

				if (ImGui::BeginCombo("Type", currentLightTypeString))
				{
					for (uint8_t i = 0u; i < ARRAYSIZE(lightTypeStrings); i++)
					{
						bool isSelected = currentLightTypeString == lightTypeStrings[i];
						if (ImGui::Selectable(lightTypeStrings[i], isSelected))
						{
							if (!isSelected)
							{
								auto color = lc.Color;
								auto intensity = lc.Intensity;
								m_pScene->GetEntityManager().Remove<PointLightComponent>(m_SelectedEntity);
								m_pScene->GetLightManager().DeallocatePointLight(m_SelectedEntity);
								auto& dlc = m_pScene->GetEntityManager().Add<DirectionalLightComponent>(m_SelectedEntity);
								m_pScene->GetLightManager().AllocateDirectionalLight(m_SelectedEntity);
								dlc.Color = color;
								dlc.Intensity = intensity;
								m_pScene->GetEntityManager().AddOrReplace<DirtyLightComponent>(m_SelectedEntity);

								m_pScene->GetEntityManager().Get<TransformComponent>(m_SelectedEntity).Rotation = DirectX::XMFLOAT3(50.0f, -30.0f, 0.0f);
							}
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				if (ImGui::ColorEdit3("Color", &lc.Color.x))
					m_pScene->GetEntityManager().AddOrReplace<DirtyLightComponent>(m_SelectedEntity);

				if (ImGui::DragFloat("Intensity", &lc.Intensity, 0.05f, 0.0f, 1.0f))
					m_pScene->GetEntityManager().AddOrReplace<DirtyLightComponent>(m_SelectedEntity);
			});

		DrawComponentNode<MeshRendererComponent>("Mesh Renderer", [this]()
			{
				auto& mrc = m_pScene->GetEntityManager().Get<MeshRendererComponent>(m_SelectedEntity);
				ImGui::ColorEdit3("Color", &mrc.Color.x);
			});

		DrawComponentNode<CameraComponent>("Camera", [this]()
			{
				auto& cc = m_pScene->GetEntityManager().Get<CameraComponent>(m_SelectedEntity);
				ImGui::DragFloat("Field of View", &cc.FieldOfViewDegrees, 1.0f, 1.0f, 179.0f);
				ImGui::DragFloat("Near Clipping Plane", &cc.ClippingPlaneNear, 0.03f, 0.01f, cc.ClippingPlaneFar);
				ImGui::DragFloat("Far Clipping Plane", &cc.ClippingPlaneFar, 0.03f, cc.ClippingPlaneNear);
				ImGui::Checkbox("Main Camera", &cc.IsMainCamera);
			});

		ImGui::Separator();
		const ImVec2 padding = ImGui::CalcTextSize("Add Component");
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f - (padding.x / 2.0f));

		static bool alreadyHasComponent = false;
		static std::string componentString = "";
		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("Add Component Settings");
		}
		if (ImGui::BeginPopup("Add Component Settings"))
		{
			if (ImGui::MenuItem("Mesh Renderer"))
			{
				if (!m_pScene->GetEntityManager().Has<MeshRendererComponent>(m_SelectedEntity))
					m_pScene->GetEntityManager().Add<MeshRendererComponent>(m_SelectedEntity);
				else
				{
					alreadyHasComponent = true;
					componentString = "MeshRenderer";
				}
			}
			if (ImGui::BeginMenu("Light"))
			{
				if (ImGui::MenuItem("Directional"))
				{
					if (!m_pScene->GetEntityManager().Has<DirectionalLightComponent>(m_SelectedEntity) &&
						!m_pScene->GetEntityManager().Has<PointLightComponent>(m_SelectedEntity))
					{
						m_pScene->GetEntityManager().Add<DirectionalLightComponent>(m_SelectedEntity);
						m_pScene->GetLightManager().AllocateDirectionalLight(m_SelectedEntity);
					}
					else
					{
						alreadyHasComponent = true;
						componentString = "Light";
					}
				}
				if (ImGui::MenuItem("Point"))
				{
					if (!m_pScene->GetEntityManager().Has<DirectionalLightComponent>(m_SelectedEntity) &&
						!m_pScene->GetEntityManager().Has<PointLightComponent>(m_SelectedEntity))
					{
						m_pScene->GetEntityManager().Add<PointLightComponent>(m_SelectedEntity);
						m_pScene->GetLightManager().AllocatePointLight(m_SelectedEntity);
					}
					else
					{
						alreadyHasComponent = true;
						componentString = "Light";
					}
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		if (alreadyHasComponent)
		{
			ImGui::OpenPopup("Can't add the same component multiple times!");
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal("Can't add the same component multiple times!", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
			{
				ImGui::Text("The component %s can't be added because \n%s already contains the same component.\n\n\n", componentString.c_str(), m_pScene->GetEntityManager().Get<NameComponent>(m_SelectedEntity).Name.c_str());
				ImGui::Separator();
				const ImVec2 buttonTextSize = ImGui::CalcTextSize("Cancel");
				ImGui::SetCursorPosX((ImGui::GetWindowContentRegionWidth() / 2.0f) - (buttonTextSize.x / 2.0f));
				
				if (ImGui::Button("Cancel"))
				{
					alreadyHasComponent = false;
					componentString = "";
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

	}

	template<typename ComponentType, typename ContextFunction>
	void PropertiesPanel::DrawComponentNode(const char* nodeName,const ContextFunction&& func) noexcept
	{
		if (!m_pScene->GetEntityManager().Has<ComponentType>(m_SelectedEntity))
			return;

		ImGui::Separator();
		constexpr const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		const bool opened = ImGui::TreeNodeEx((void*)typeid(ComponentType).hash_code(), flags, nodeName);

		const ImVec2 size = ImGui::CalcTextSize("...");
		ImGui::SameLine(ImGui::GetWindowContentRegionWidth() - (size.x / 2.0f));

		bool removeComponent = false;
		if (ImGui::Button("..."))
		{
			ImGui::OpenPopup("Component Settings");
		}
		if (ImGui::BeginPopup("Component Settings"))
		{
			if (ImGui::MenuItem("Remove"))
			{
				removeComponent = true;
			}
			ImGui::EndPopup();
		}

		if (opened)
		{
			func();
			ImGui::TreePop();
		}

		if (removeComponent)
			m_pScene->GetEntityManager().Remove<ComponentType>(m_SelectedEntity);
	}

	bool PropertiesPanel::DrawVec3Control(const char* label, DirectX::XMFLOAT3& values, float dragSpeed, float resetValue, float minValue, float maxValue, float columnWidth) noexcept
	{
		bool changedValues{ false };

		ImGui::PushID(label);
		auto pFont = ImGui::GetIO().Fonts->Fonts[OPENSANS_BOLD_18];

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		constexpr float buttonPaddingOffsetX = 3.0f;
		ImVec2 buttonSize = { lineHeight + buttonPaddingOffsetX, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.2f, 0.25f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.8f, 0.1f, 0.15f, 1.0f));
		ImGui::PushFont(pFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		changedValues |= ImGui::DragFloat("##X", &values.x, dragSpeed);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
		ImGui::PushFont(pFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		changedValues |= ImGui::DragFloat("##Y", &values.y, dragSpeed);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.35f, 0.9f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.1f, 0.25f, 0.8f, 1.0f));
		ImGui::PushFont(pFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		changedValues |= ImGui::DragFloat("##Z", &values.z, dragSpeed);
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		values.x = std::clamp(values.x, minValue, maxValue);
		values.y = std::clamp(values.y, minValue, maxValue);
		values.z = std::clamp(values.z, minValue, maxValue);

		return changedValues;
	}
}