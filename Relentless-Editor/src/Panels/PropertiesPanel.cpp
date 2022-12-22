#include "PropertiesPanel.h"
namespace Relentless
{
	PropertiesPanel::PropertiesPanel() noexcept
		: m_SelectedEntity{ NULL_ENTITY },
		  m_pEntityManager{ nullptr }
	{
	}

	void PropertiesPanel::OnImGuiRender() noexcept
	{
		ImGui::Begin("Properties Panel");

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

	void PropertiesPanel::SetEntityManager(EntityManager* const entityManager) noexcept
	{
		RLS_ASSERT(entityManager, "Entity manager is nullptr.");
		m_pEntityManager = entityManager;
	}

	void PropertiesPanel::DrawAllComponentNodes()
	{
		auto& nc = m_pEntityManager->Get<NameComponent>(m_SelectedEntity);

		if (m_FormattingName == false)
		{
			bool shouldRender = m_pEntityManager->Has<ForwardPassComponent>(m_SelectedEntity);
			char buff[33];
			buff[0] = buff[1] = '#';
			char* bPtr = buff;
			bPtr += 2;
#pragma warning(push, 0)
			_itoa(m_SelectedEntity, bPtr, 2);
#pragma warning(pop)			
			if (ImGui::Checkbox(buff, &shouldRender))
			{
				if (!shouldRender)
					m_pEntityManager->Remove<ForwardPassComponent>(m_SelectedEntity);
				else
					m_pEntityManager->Add<ForwardPassComponent>(m_SelectedEntity);
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

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Framed;
		bool opened = ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), flags, "Transform");

		if (opened)
		{
			auto& tc = m_pEntityManager->Get<TransformComponent>(m_SelectedEntity);
			float translation[3];
			float scale[3];
			float rotation[3];
			translation[0] = tc.Translation.x;
			translation[1] = tc.Translation.y;
			translation[2] = tc.Translation.z;
			rotation[0] = tc.Rotation.x;
			rotation[1] = tc.Rotation.y;
			rotation[2] = tc.Rotation.z;
			scale[0] = tc.Scale.x;
			scale[1] = tc.Scale.y;
			scale[2] = tc.Scale.z;


			if (ImGui::DragFloat3("Position", translation, 0.06f))
			{
				tc.Translation.x = translation[0]; 
				tc.Translation.y = translation[1];
				tc.Translation.z = translation[2];

				DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&tc.Scale)) * DirectX::XMMatrixRotationX(0) * DirectX::XMMatrixRotationY(0) * DirectX::XMMatrixRotationZ(0) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&tc.Translation));
			
				DirectX::XMStoreFloat4x4(&tc.Transform, world);
			}
			if (ImGui::DragFloat3("Rotation", rotation, 0.03f))
			{
				tc.Rotation.x = rotation[0];
				tc.Rotation.y = rotation[1];
				tc.Rotation.z = rotation[2];

				float angleRadX = DirectX::XMConvertToRadians(tc.Rotation.x);
				float angleRadY = DirectX::XMConvertToRadians(tc.Rotation.y);
				float angleRadZ = DirectX::XMConvertToRadians(tc.Rotation.z);

				DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&tc.Scale)) * DirectX::XMMatrixRotationX(angleRadX) * DirectX::XMMatrixRotationY(angleRadY) * DirectX::XMMatrixRotationZ(angleRadZ) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&tc.Translation));
				
				DirectX::XMStoreFloat4x4(&tc.Transform, world);
			}
			if (ImGui::DragFloat3("Scale", scale, 0.03f))
			{
				tc.Scale.x = std::max(0.01f, scale[0]);
				tc.Scale.y = std::max(0.01f, scale[1]);
				tc.Scale.z = std::max(0.01f, scale[2]);

				DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&tc.Scale)) * DirectX::XMMatrixRotationX(0) * DirectX::XMMatrixRotationY(0) * DirectX::XMMatrixRotationZ(0) * DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&tc.Translation));

				DirectX::XMStoreFloat4x4(&tc.Transform, world);
			}

			ImGui::TreePop();
		}

		if (m_pEntityManager->Has<LightComponent>(m_SelectedEntity))
		{
			opened = ImGui::TreeNodeEx((void*)typeid(LightComponent).hash_code(), flags, "Light");
			if (opened)
			{
				auto& lc = m_pEntityManager->Get<LightComponent>(m_SelectedEntity);

				constexpr const char* lightTypeStrings[] = { "Directional" };
				const char* currentLightTypeString = lightTypeStrings[(int)lc.LightType];

				if (ImGui::BeginCombo("Type", currentLightTypeString))
				{
					for (uint8_t i = 0u; i < ARRAYSIZE(lightTypeStrings); i++)
					{
						bool isSelected = currentLightTypeString == lightTypeStrings[i];
						if (ImGui::Selectable(lightTypeStrings[i], isSelected))
						{
							//...
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				ImGui::Text("All lighting is placeholder and WIP.");
				ImGui::TreePop();
			}
		}

	}
}