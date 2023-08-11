#include "ContentBrowserPanel.h"
namespace Relentless
{
	static std::filesystem::path currentDirectory = EDITOR_ASSET_DIRECTORY;

	ContentBrowserPanel::ContentBrowserPanel() noexcept
		: m_ThumbnailWidth{100.0f},
		  m_SelectedDirectory{"Assets"}
	{
		m_pDirectoryTexture = Texture2D::Create("Directory.jpg");
		m_pSceneTexture = Texture2D::Create("RLS_LOGO.jpg");
		m_pMaterialTexture = Texture2D::Create("FileIcon.png");

		std::string directory = currentDirectory.string();
		currentDirectory = directory.substr(0u, directory.size() - 1);
	}

	void ContentBrowserPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;

		PROFILE_FUNC;

		RenderMenuBar();

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, m_ThumbnailWidth * 2.0f);
		
		RenderDirectoryHierarchySearchBox();
		RenderDirectoryHierarchy();

		ImGui::NextColumn();

		RenderAssetHierarchyOverview();
		RenderAssetSearchBox();
		RenderAssetThumbNails();
		RenderPopUpOptions();
		
		ImGui::Columns(1);

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
		{
			m_OnAssetSelectedCallback(NULL_RESOURCEID, InspectedAssetType::NONE);
		}

		ImGui::End();
	}

	void ContentBrowserPanel::SetOnAssetSelectedCallback(std::function<void(const ResourceID& resourceID, const InspectedAssetType inspectedAssetType)> callback) noexcept
	{
		m_OnAssetSelectedCallback = callback;
	}

	void ContentBrowserPanel::RenderDirectoryHierarchy() noexcept
	{
		ImGuiTreeNodeFlags sceneNodeflags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;
		sceneNodeflags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
		ImGui::PushFont(boldFont);
		bool opened = ImGui::TreeNodeEx("Content", sceneNodeflags, "Content");
		ImGui::PopFont();

		if (opened)
		{
			for (auto const& directoryEntry : std::filesystem::directory_iterator{ EDITOR_ASSET_DIRECTORY })
			{
				if (directoryEntry.is_directory())
				{
					DrawDirectoryNode(directoryEntry);
				}
			}
			ImGui::TreePop();
		}
	}

	void ContentBrowserPanel::DrawDirectoryNode(const std::filesystem::directory_entry& directoryEntry) noexcept
	{
		const std::string& directoryEntryName = directoryEntry.path().filename().string();

		ImGuiTreeNodeFlags directoryNodeflags = ((directoryEntryName == m_SelectedDirectory) ? ImGuiTreeNodeFlags_Selected : 0);
		directoryNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;
		directoryNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow;

		ImGuiStyle* pStyle = &ImGui::GetStyle();
		pStyle->Alpha = (directoryEntryName == m_SelectedDirectory) ? 1.0f : 0.5f;

		bool opened = ImGui::TreeNodeEx(directoryEntryName.c_str(), directoryNodeflags, directoryEntryName.c_str());
		if (ImGui::IsItemClicked() && directoryEntryName != m_SelectedDirectory)
		{
			m_SelectedDirectory = directoryEntryName;
			currentDirectory = directoryEntry;
		}

		if (opened)
		{
			for (auto const& nestedDirectoryEntry : std::filesystem::directory_iterator{ directoryEntry })
			{
				if (nestedDirectoryEntry.is_directory())
				{
					DrawDirectoryNode(nestedDirectoryEntry);
				}
			}
		}

		if (opened)
			ImGui::TreePop();

		pStyle->Alpha = 1.0f;
	}

	void ContentBrowserPanel::RenderMenuBar() noexcept
	{
		ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar())
		{
			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
			ImGui::PushFont(boldFont);
			if (ImGui::Button("<"))
			{
				if (m_SelectedDirectory != "Assets")
				{
					currentDirectory = currentDirectory.parent_path();
					m_SelectedDirectory = currentDirectory.filename().string();
				}
			}
			if (ImGui::Button(">"))
			{
				//TODO
			}
			ImGui::PopFont();

			m_LocationStringPosition[1] = ImGui::GetCursorScreenPos().y + ImGui::GetFrameHeight() / 8.0f;

			ImGui::EndMenuBar();
		}
	}

	void ContentBrowserPanel::RenderDirectoryHierarchySearchBox() noexcept
	{
		ImGui::PushItemWidth(ImGui::GetColumnWidth() - ImGui::GetStyle().WindowPadding.x - 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
		static char searchBuf[256] = "";
		ImGui::InputTextWithHint("##Search", "Search Folders...", searchBuf, IM_ARRAYSIZE(searchBuf));
		ImGui::PopStyleVar();
	}

	void ContentBrowserPanel::RenderAssetHierarchyOverview() noexcept
	{
		const size_t offset = currentDirectory.string().find("Assets");
		std::string displayLocationString = currentDirectory.string().substr(offset);
		for (auto& character : displayLocationString)
		{
			if (character == '\\')
				character = '/';
		}

		std::vector<std::string> result;
		std::istringstream iss(displayLocationString);
		std::string token;

		std::string finalString;
		while (std::getline(iss, token, '/')) {
			result.push_back(token);
			finalString = finalString + token;
			finalString = finalString + " > ";
		}

		ImDrawList* pDrawList = ImGui::GetForegroundDrawList();
		ImVec2 textPosition(m_LocationStringPosition[0], m_LocationStringPosition[1]);

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
		ImGui::PushFont(boldFont);

		pDrawList->AddText(textPosition, ImColor(255, 255, 255, 255), finalString.c_str());
		ImGui::PopFont();

	}

	void ContentBrowserPanel::RenderAssetSearchBox() noexcept
	{
		static char searchBuf2[256] = "";
		ImGui::PushItemWidth(ImGui::GetColumnWidth() - ImGui::GetStyle().WindowPadding.x - 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
		ImGui::InputTextWithHint("##AssetSearch", "Search Assets...", searchBuf2, IM_ARRAYSIZE(searchBuf2));
		ImGui::PopStyleVar();
	}

	void ContentBrowserPanel::RenderAssetThumbNails() noexcept
	{
		ImGui::BeginTable("ContentBrowserTable", 8, ImGuiTableFlags_::ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_::ImGuiTableFlags_PreciseWidths);

		m_LocationStringPosition[0] = ImGui::GetCursorScreenPos().x;
		for (auto const& dir_entry : std::filesystem::directory_iterator{ currentDirectory })
		{
			std::string entryPath = dir_entry.path().string();
			std::string entry = dir_entry.path().filename().string();

			if (dir_entry.is_directory())
			{
				ImGui::TableNextColumn();

				ImGui::ImageButton((void*)m_pDirectoryTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth), ImVec2(0, 0), ImVec2(1, 1), -1, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				{
					currentDirectory /= entry;
					m_SelectedDirectory = entry;
				}
				RenderThumbnailText(entry, ImGui::IsItemHovered());
			}
			else
			{
				ImGui::TableNextColumn();

				if (dir_entry.path().filename().extension().string() == ".jpg" || dir_entry.path().filename().extension().string() == ".png")
				{
					if (!AssetManager::Get().HasLoaded(dir_entry.path().string()))
					{
						AssetManager::Get().Load<Texture2D>(dir_entry.path().string());
					}

					ResourceID textureResourceID = AssetManager::Get().Load<Texture2D>(dir_entry.path().string());
					Texture2D* pTexture = AssetManager::Get().GetAsset<Texture2D>(textureResourceID);
					ImGui::ImageButton((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));

					if (ImGui::BeginDragDropSource())
					{
						const char* path = entryPath.c_str();
						ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
						ImGui::BeginTooltip();
						ImGui::Image((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));
						ImGui::EndTooltip();
						ImGui::PopStyleVar(1);
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_TEXTURE", path, strlen(path) + 1, ImGuiCond_::ImGuiCond_Once);
						ImGui::EndDragDropSource();
					}

					ImGui::SameLine();
					ImGui::NewLine();

					RenderThumbnailText(pTexture->GetName(), ImGui::IsItemHovered());
				}
				else if (dir_entry.path().filename().extension().string() == ".Relentless")
				{
					ImGui::ImageButton((ImTextureID)m_pSceneTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));

					if (ImGui::BeginDragDropSource())
					{
						const char* path = entryPath.c_str();
						ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
						ImGui::BeginTooltip();
						ImGui::Image((ImTextureID)m_pSceneTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));
						ImGui::EndTooltip();
						ImGui::PopStyleVar(1);
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_SCENE", path, strlen(path) + 1, ImGuiCond_::ImGuiCond_Once);
						ImGui::EndDragDropSource();
					}

					ImGui::SameLine();
					ImGui::NewLine();

					RenderThumbnailText(dir_entry.path().filename().string(), ImGui::IsItemHovered());
				}
			}
		}

		auto it = m_CreatedMaterials.find(currentDirectory.string());
		if (it != m_CreatedMaterials.end())
		{
			for (auto& materialHandle : it->second)
			{
				ImGui::TableNextColumn();

				ImGui::ImageButton((ImTextureID)m_pMaterialTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					m_OnAssetSelectedCallback(materialHandle, InspectedAssetType::MATERIAL);
				}

				if (ImGui::BeginDragDropSource())
				{
					ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
					ImGui::BeginTooltip();
					ImGui::Image((ImTextureID)m_pMaterialTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));
					ImGui::EndTooltip();
					ImGui::PopStyleVar(1);
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_MATERIAL", (void*)&materialHandle, sizeof(Material), ImGuiCond_::ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				

				ImGui::SameLine();
				ImGui::NewLine();
				RenderThumbnailText(AssetManager::Get().Get<Material>(materialHandle).GetName(), ImGui::IsItemHovered());
			}
		}

		ImGui::EndTable();
	}

	void ContentBrowserPanel::RenderPopUpOptions() noexcept
	{
		if (ImGui::BeginPopupContextWindow(0, 1, false /*over items*/))
		{
			if (ImGui::MenuItem("Import New Asset..."))
			{
				std::filesystem::path filePath = FileDialogs::OpenFile("Image files (*.jpg,*.png)\0*.jpg;*.png\0");
				std::string extension = filePath.extension().string();

				if (extension == ".jpg" || extension == ".png")
				{
					if (!AssetManager::Get().HasLoaded(filePath.string()))
					{
						std::filesystem::copy(filePath, currentDirectory, std::filesystem::copy_options::overwrite_existing);
						AssetManager::Get().Load<Texture2D>(filePath.string());
					}
				}
			}
			if (ImGui::MenuItem("New Material"))
			{
				m_CreatedMaterials[currentDirectory.string()].push_back(AssetManager::Get().Create<Material>("New Material"));
			}
			ImGui::EndPopup();
		}
	}

	void ContentBrowserPanel::RenderThumbnailText(const std::string& text, bool thumbNailHovered) noexcept
	{
		ImGui::PushItemWidth(m_ThumbnailWidth);

		std::string textToRender = text;
		ImVec2 textSize = ImGui::CalcTextSize(textToRender.c_str());
		// Calculate indent for center alignment
		constexpr float paddingAdjustment = 5.0f;
		float indent = (m_ThumbnailWidth - textSize.x) / 2.0f - paddingAdjustment;

		// If text is too long, truncate it and add "..."
		if (textSize.x > m_ThumbnailWidth)
		{
			// calculate the number of chars that can be fit in the width
			uint32_t nrOfAllowedCharacters = static_cast<uint32_t>((m_ThumbnailWidth / textSize.x) * text.length());
			const float ellipsisWidth = ImGui::CalcTextSize("...").x;

			// take into account the width of "..." when truncating the text
			while (ImGui::CalcTextSize(text.substr(0, nrOfAllowedCharacters).c_str()).x + ellipsisWidth > m_ThumbnailWidth)
			{
				nrOfAllowedCharacters -= 1;
			}

			textToRender = textToRender.substr(0, nrOfAllowedCharacters) + "...";
			textSize = ImGui::CalcTextSize(textToRender.c_str());
			indent = (m_ThumbnailWidth - textSize.x) / 2.0f - paddingAdjustment;
		}

		// Set indent
		ImGui::Dummy(ImVec2(indent, 0)); // Dummy is used to create an empty space, effectively indenting the text
		ImGui::SameLine();

		// Display the text
		ImGui::Selectable(textToRender.c_str(), thumbNailHovered, 0, textSize);

		ImGui::PopItemWidth();
	}
}