#include "ContentBrowserPanel.h"
namespace Relentless
{
	static std::filesystem::path currentDirectory = "C:/Users/emilf/Desktop/RelentlessD12/Relentless/Assets/Textures";

	void ContentBrowserPanel::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		ImGui::Begin("Content browser Panel");
		
		ImGui::BeginTable("ContentBrowserTable", 8, ImGuiTableFlags_::ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_::ImGuiTableFlags_PreciseWidths);
		for (auto const& dir_entry : std::filesystem::directory_iterator{ currentDirectory })
		{
			std::string entry = dir_entry.path().filename().string();

			if (dir_entry.is_directory())
			{
				ImGui::TableNextColumn();

				//ImGui::ImageButton((void*)m_pDirectoryIconTexture->GetShaderResourceView().Get(), ImVec2(100.0f, 100.0f), ImVec2(0, 0), ImVec2(1, 1), -1, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
				//if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				//	currentDirectory /= entry;

				float offset = (ImGui::GetColumnWidth() - ImGui::CalcTextSize(entry.c_str()).x + ImGui::GetFontSize()) * 0.5f + (ImGui::TableGetColumnIndex() * ImGui::GetColumnWidth()) + (ImGui::TableGetColumnIndex() * 8.5f);
				ImGui::SetCursorPosX(offset);
				ImGui::Text("%s", entry.c_str());
			}
			else
			{
				ImGui::TableNextColumn();

				if (dir_entry.path().filename().extension().string() == ".jpg" || dir_entry.path().filename().extension().string() == ".png")
				{
					if (!AssetManager::Get().HasLoaded(entry))
					{
						AssetManager::Get().Load<Texture2D>(entry);
					}
				}

				ResourceID textureResourceID = AssetManager::Get().Load<Texture2D>(entry);
				Texture2D* pTexture = AssetManager::Get().GetAsset<Texture2D>(textureResourceID);
				ImGui::ImageButton((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(100.0f, 100.0f));

				if (ImGui::BeginDragDropSource())
				{
					const char* path = entry.c_str();
					ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
					ImGui::BeginTooltip();
					ImGui::Image((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(100.0f, 100.0f));
					ImGui::EndTooltip();
					ImGui::PopStyleVar(1);
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_TEXTURE", path, strlen(path) + 1, ImGuiCond_::ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

			}
		}
		ImGui::EndTable();



























		if (ImGui::BeginPopupContextWindow(0, 1, false /*over items*/))
		{
			if (ImGui::MenuItem("Import New Asset..."))
			{
				std::filesystem::path filePath = FileDialogs::OpenFile("Image files (*.jpg,*.png)\0*.jpg;*.png\0");
				std::string extension = filePath.extension().string();
				RLS_CORE_INFO("{0}", extension);

				if (extension == ".jpg" || extension == ".png")
				{
					if (!AssetManager::Get().HasLoaded(filePath.filename().string()))
					{
						AssetManager::Get().Load<Texture2D>(filePath.filename().string());
					}
				}

			}

			ImGui::EndPopup();
		}


		ImGui::End();
	}
}