#include "InspectorPanel.h"

namespace Relentless
{
	InspectorPanel::InspectorPanel() noexcept
		: m_InspectedAssetHandle{ NULL_HANDLE },
		  m_InspectedAssetType{ InspectedAssetType::NONE },
		  m_ForceDisplay{false},
		  m_MaterialMapThumbnailSize{25.0f}
	{
		m_pColorPickerWidgetTexture = Texture2D::Create(std::string(ENGINE_ASSET_DIRECTORY) + "Textures/pickerwidget.png");
	}

	void InspectorPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;

		if (m_ForceDisplay)
		{
			ImGui::SetNextWindowFocus();
			m_ForceDisplay = false;
		}

		ImGui::Begin("Inspector");

		switch (m_InspectedAssetType)
		{
		case InspectedAssetType::MATERIAL:
			RenderMaterialInspector();
			break;
		}

		ImGui::End();
	}

	void InspectorPanel::SetContext(const AssetHandle& assetHandle, const InspectedAssetType assetType) noexcept
	{
		m_InspectedAssetHandle = assetHandle;
		m_InspectedAssetType = assetType;
		if (m_InspectedAssetHandle != NULL_HANDLE)
			m_ForceDisplay = true;
	}

	void InspectorPanel::RenderMaterialInspector() noexcept
	{
		RLS_ASSERT(m_InspectedAssetHandle != NULL_HANDLE, "Asset handle is invalid.");
		RLS_ASSERT(m_InspectedAssetType != InspectedAssetType::NONE, "Asset type is invalid.");

		if (m_InspectedAssetHandle != NULL_HANDLE)
		{
			std::shared_ptr<Material> material = AssetManager::Get<Material>(m_InspectedAssetHandle);

			bool isDefaultMaterial = material->GetName() == "Default-Material";
			if (isDefaultMaterial)
			{
				ImGui::PushItemFlag(ImGuiItemFlags_::ImGuiItemFlags_Disabled, true);
			}

			bool changedMaterial = false;

			ImGuiIO& io = ImGui::GetIO();
			auto font = io.Fonts->Fonts[1];
			ImGui::PushFont(font);
			std::string textToDisplay = material->GetName() + " (Material)";
			ImGui::Text(textToDisplay.c_str());
			ImGui::PopFont();

			ImGui::Separator();

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 120.0f);
			ImGui::SetColumnWidth(1, 297.0f);
			ImGui::Text("Render Mode");
			ImGui::NextColumn();
			
			ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

			const char* renderModeStrings[] = { "None", "Opaque", "CutOut", "Transparent"};
			const char* currentRenderModeString = renderModeStrings[(uint32_t)material->GetRenderMode()];

			if (ImGui::BeginCombo("##RenderModeString", currentRenderModeString))
			{
				for (uint8_t i = 0u; i < ARRAYSIZE(renderModeStrings); ++i)
				{
					bool isSelected = currentRenderModeString == renderModeStrings[i];
					if (ImGui::Selectable(renderModeStrings[i], isSelected))
					{
						if (!isSelected)
						{
							material->SetRenderMode((RenderMode)i);
						}
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::PopItemWidth();
			ImGui::PopStyleVar();

			bool combined = material->m_CombinedRoughnessMetallnesMap;
			if (ImGui::Checkbox("Combined Roughness & Metalness", &combined))
			{
				material->m_CombinedRoughnessMetallnesMap = combined;
				changedMaterial = true;
			}

			ImGui::Columns(1);
			ImGui::Separator();

			auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
			ImGui::PushFont(boldFont);
			ImGui::Text("Maps");
			ImGui::PopFont();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 120.0f);
			ImGui::SetColumnWidth(1, 297.0f);

			if (material->HasAlbedoTexture())
			{
				std::shared_ptr<Texture2D> albedoTexture = material->GetAlbedoTexture();
				ImGui::ImageButton((ImTextureID)albedoTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetAlbedoTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				else
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(albedoTexture->GetName().c_str());
						ImGui::EndTooltip();
					}
				}

				ImGui::SameLine();
				ImGui::Text("Albedo");
				if (ImGui::BeginPopupContextItem("AlbedoContextMenu"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						material->RemoveAlbedoTexture();
						changedMaterial = true;
					}
					ImGui::EndPopup();
				}
			}
			else
			{
				static ImVec4 buttonColor = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
				static ImGuiColorEditFlags buttonFlags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop;
				ImGui::ColorButton("##MyColor", buttonColor, buttonFlags, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetAlbedoTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				ImGui::Text("Albedo");
			}

			ImGui::NextColumn();

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, ImGui::GetStyle().ItemSpacing.y));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, ImGui::GetStyle().ItemInnerSpacing.y));

			float yOffset = 1.0f;
			ImVec2 cursorPos = ImGui::GetCursorPos(); // Get the current cursor position
			cursorPos.y += yOffset; // Add your desired offset
			ImGui::SetCursorPos(cursorPos); // Set the new cursor position

			for (uint8_t i{ 1u }; i < 8; ++i)
			{
				std::string name = "##Color" + std::to_string(i);
				changedMaterial |= ImGui::ColorEdit4(name.c_str(), &material->m_AlbedoColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoBorder | ImGuiColorEditFlags_::ImGuiColorEditFlags_HDR);
				ImGui::SameLine();
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::ImageButton((ImTextureID)m_pColorPickerWidgetTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImGui::GetItemRectSize(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
			ImGui::PopStyleColor();

			ImGui::PopStyleVar(2);

			ImGui::Columns(1);

			ImGui::Columns(2);
			
			if (material->HasMetallicTexture())
			{
				std::shared_ptr<Texture2D> metallicTexture = material->GetMetallicTexture();
				ImGui::ImageButton((ImTextureID)metallicTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetMetallicTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				else
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(metallicTexture->GetName().c_str());
						ImGui::EndTooltip();
					}
				}

				ImGui::SameLine();
				ImGui::Text("Metallic");
				if (ImGui::BeginPopupContextItem("MetallicContextMenu"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						material->RemoveMetallicTexture(); 
						changedMaterial = true;
					}
					ImGui::EndPopup();
				}
			}
			else
			{
				static ImVec4 buttonColor = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
				static ImGuiColorEditFlags buttonFlags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop;
				ImGui::ColorButton("##MyColor3", buttonColor, buttonFlags, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetMetallicTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				ImGui::Text("Metallic");
			}
			
			ImGui::NextColumn();
			
			changedMaterial |= ImGui::DragFloat("##Metallic", &material->m_Metallic, 0.006f, 0.0f, 1.0f);

			ImGui::Columns(1);
		
			ImGui::Columns(2);
			
			if (material->HasRoughnessTexture())
			{
				std::shared_ptr<Texture2D> roughnessTexture = material->GetRoughnessTexture();
				ImGui::ImageButton((ImTextureID)roughnessTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetRoughnessTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				else
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(roughnessTexture->GetName().c_str());
						ImGui::EndTooltip();
					}
				}

				ImGui::SameLine();
				ImGui::Text("Roughness");
				if (ImGui::BeginPopupContextItem("RoughnessContextMenu"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						material->RemoveRoughnessTexture();
						changedMaterial = true;
					}
					ImGui::EndPopup();
				}
			}
			else
			{
				static ImVec4 buttonColor = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
				static ImGuiColorEditFlags buttonFlags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop;
				ImGui::ColorButton("##MyColor4", buttonColor, buttonFlags, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetRoughnessTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}

				ImGui::SameLine();
				ImGui::Text("Roughness");
			}

			ImGui::NextColumn();

			changedMaterial |= ImGui::DragFloat("##Roughness", &material->m_Roughness, 0.006f, 0.0f, 1.0f);

			ImGui::Columns(1);
	
			ImGui::Columns(2);

			if (material->HasNormalMap())
			{
				std::shared_ptr<Texture2D> normalMap = material->GetNormalMap();
				ImGui::ImageButton((ImTextureID)normalMap->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetNormalMap(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				else
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(normalMap->GetName().c_str());
						ImGui::EndTooltip();
					}
				}

				ImGui::SameLine();
				ImGui::Text("Normal");
				if (ImGui::BeginPopupContextItem("NormalContextMenu"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						material->RemoveNormalMap();
						changedMaterial = true;
					}
					ImGui::EndPopup();
				}
			}
			else
			{
				static ImVec4 buttonColor = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
				static ImGuiColorEditFlags buttonFlags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop;
				ImGui::ColorButton("##MyColor15", buttonColor, buttonFlags, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetNormalMap(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				ImGui::Text("Normal");
			}
			

			ImGui::NextColumn();
			ImGui::Columns(1);


			ImGui::Columns(2);

			if (material->HasHeightMap())
			{
				std::shared_ptr<Texture2D> heightMap = material->GetHeightMap();
				ImGui::ImageButton((ImTextureID)heightMap->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetHeightMap(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				else
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(heightMap->GetName().c_str());
						ImGui::EndTooltip();
					}
				}

				ImGui::SameLine();
				ImGui::Text("Height");
				if (ImGui::BeginPopupContextItem("HeightContextMenu"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						material->RemoveHeightMap();
						changedMaterial = true;
					}
					ImGui::EndPopup();
				}
			}
			else
			{
				static ImVec4 buttonColor = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
				static ImGuiColorEditFlags buttonFlags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop;
				ImGui::ColorButton("##MyColor88", buttonColor, buttonFlags, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetHeightMap(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				ImGui::Text("Height");
			}
			

			ImGui::NextColumn();

			if (material->HasHeightMap())
			{
				changedMaterial |= ImGui::DragFloat("##HeightScale", &material->m_HeightScale, 0.0004f, 0.005f, 0.08f);
			}

			ImGui::Columns(1);

			ImGui::Columns(2);
			
			if (material->HasAmbientOcclusionTexture())
			{
				std::shared_ptr<Texture2D> ambientOcclusionTexture = material->GetAmbientOcclusionTexture();
				ImGui::ImageButton((ImTextureID)ambientOcclusionTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetAmbientOcclusionTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				else
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(ambientOcclusionTexture->GetName().c_str());
						ImGui::EndTooltip();
					}
				}

				ImGui::SameLine();
				ImGui::Text("AO");
				if (ImGui::BeginPopupContextItem("AOContextMenu"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						material->RemoveAmbientOcclusionTexture();
						changedMaterial = true;
					}
					ImGui::EndPopup();
				}
			}
			else
			{
				static ImVec4 buttonColor = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
				static ImGuiColorEditFlags buttonFlags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop;
				ImGui::ColorButton("##MyColor5", buttonColor, buttonFlags, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetAmbientOcclusionTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				ImGui::Text("AO");
			}

			ImGui::NextColumn();
		
			if (material->HasAmbientOcclusionTexture())
			{
				changedMaterial |= ImGui::DragFloat("##AOScale", &material->m_AOScale, 0.006f, 0.0f, 1.0f);
			}

			ImGui::Columns(3);
			ImGui::SetColumnWidth(0, 120.0f);
			
			if (material->HasEmissionTexture())
			{
				std::shared_ptr<Texture2D> emissionTexture = material->GetEmissionTexture();
				ImGui::ImageButton((ImTextureID)emissionTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("TEXTURE2D_DRAG_DROP"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetEmissionTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				else
				{
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(emissionTexture->GetName().c_str());
						ImGui::EndTooltip();
					}
				}

				ImGui::SameLine();
				ImGui::Text("Emission");
				if (ImGui::BeginPopupContextItem("EmissionContextMenu"))
				{
					if (ImGui::MenuItem("Reset"))
					{
						material->RemoveEmissionTexture();
						changedMaterial = true;
					}
					ImGui::EndPopup();
				}
			}
			else
			{
				static ImVec4 buttonColor = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
				static ImGuiColorEditFlags buttonFlags = ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoDragDrop;
				ImGui::ColorButton("##MyColor6", buttonColor, buttonFlags, ImVec2(m_MaterialMapThumbnailSize, m_MaterialMapThumbnailSize));
				if (!isDefaultMaterial && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_TEXTURE"))
					{
						AssetHandle* textureHandle = (AssetHandle*)payLoad->Data;
						material->SetEmissionTexture(*textureHandle);
						changedMaterial = true;
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				ImGui::Text("Emission");
			}

			ImGui::NextColumn();

			ImGui::SetColumnWidth(1, 110.0f);
			ImGui::SetColumnWidth(2, 120.0f);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, ImGui::GetStyle().ItemSpacing.y));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, ImGui::GetStyle().ItemInnerSpacing.y));

			yOffset = 1.0f;
			cursorPos = ImGui::GetCursorPos(); // Get the current cursor position
			cursorPos.y += yOffset; // Add your desired offset
			ImGui::SetCursorPos(cursorPos); // Set the new cursor position

			for (uint8_t i{ 1u }; i < 4; ++i)
			{
				std::string name = "##EmissionColor" + std::to_string(i);
				changedMaterial |= ImGui::ColorEdit3(name.c_str(), &material->m_EmissionColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_::ImGuiColorEditFlags_NoBorder);
				ImGui::SameLine();
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::ImageButton((ImTextureID)m_pColorPickerWidgetTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImGui::GetItemRectSize(), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0));
			ImGui::PopStyleColor();

			ImGui::PopStyleVar(2);

			ImGui::NextColumn();

			changedMaterial |= ImGui::DragFloat("##EmissionIntensity", &material->m_EmissionIntensity, 0.01f, 0.0f, FLT_MAX);
			
			ImGui::Columns(1);

			ImGui::Separator();

			changedMaterial |= DrawVec2Control("Tiling", material->m_TilingFactor, 0.06f, 1.0f, 0.0f, 100.0f, 120.0f);
			
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2.0f);
			ImGui::Separator();
			changedMaterial |= DrawVec2Control("Offset", material->m_Offset, 0.06f, 0.0f, 0.0f, 100.0f, 120.0f);
			
			if (changedMaterial)
			{
				Application::Get().GetMemorymanager().SetDirtyMaterial(m_InspectedAssetHandle);
			}

			if (material->GetName() == "Default-Material")
			{
				ImGui::PopItemFlag();
			}

		}
	}

	bool InspectorPanel::DrawVec2Control(const char* label, DirectX::XMFLOAT2& values, float dragSpeed, float resetValue, float minValue, float maxValue, float columnWidth) noexcept
	{
		bool changedValues{ false };

		ImGui::PushID(label);
		auto pFont = ImGui::GetIO().Fonts->Fonts[OPENSANS_BOLD_18];

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::SetColumnWidth(1, 215.0f);
		ImGui::Text(label);
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		const float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		constexpr float buttonPaddingOffsetX = 3.0f;
		ImVec2 buttonSize = { lineHeight + buttonPaddingOffsetX, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		ImGui::PushFont(pFont);
		if (ImGui::Button("X", buttonSize))
		{
			values.x = resetValue;
			changedValues = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		changedValues |= ImGui::DragFloat("##X", &values.x, dragSpeed);
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
		ImGui::PushFont(pFont);
		if (ImGui::Button("Y", buttonSize))
		{
			values.y = resetValue;
			changedValues = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		changedValues |= ImGui::DragFloat("##Y", &values.y, dragSpeed);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		values.x = std::clamp(values.x, minValue, maxValue);
		values.y = std::clamp(values.y, minValue, maxValue);

		return changedValues;
	}

}