#include "ContentBrowserPanel.h"
namespace Relentless
{
	static std::filesystem::path currentDirectory = EDITOR_ASSET_DIRECTORY;

	ContentBrowserPanel::ContentBrowserPanel() noexcept
		: m_ThumbnailWidth{100.0f},
		  m_SelectedDirectory{"Assets"},
		  m_AssetToName{ NULL_HANDLE }
	{
		m_DirectoryTextureHandle = AssetManager::LoadFromFile<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Directory.jpg");
		m_SceneTextureHandle = AssetManager::LoadFromFile<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\SceneThumbnail.jpg");
		m_MaterialTextureHandle = AssetManager::LoadFromFile<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\MaterialThumbnail.jpg");

		currentDirectory = currentDirectory.string().substr(0u, currentDirectory.string().size() - 1);

		//TextureImportSettings importSettings;
		//importSettings.TextureCompressionType = ETextureCompressionType::BC7_Quick;
		//aex = AssetManager::LoadFromFile<Texture2D>(EDITOR_ASSET_DIRECTORY + std::string("Textures\\bright_grid.png"), &importSettings);
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
			m_OnAssetSelectedCallback(NULL_HANDLE, InspectedAssetType::NONE);
		}

		ImGui::End();
	}

	void ContentBrowserPanel::SetOnAssetSelectedCallback(std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> callback) noexcept
	{
		m_OnAssetSelectedCallback = callback;
	}

	void ContentBrowserPanel::SetActiveScene(std::shared_ptr<Scene> pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is invalid.");
		m_pScene = pScene;
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
		ImGui::BeginTable("ContentBrowserTable", 12, ImGuiTableFlags_::ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_::ImGuiTableFlags_PreciseWidths);

		if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(0))
		{
			m_AssetToName = NULL_HANDLE;
			m_FirstTimeEditingThumbnail = true;
		}

		m_LocationStringPosition[0] = ImGui::GetCursorScreenPos().x;

		//Directories should always display first:
		for (auto const& dir_entry : std::filesystem::directory_iterator{ currentDirectory })
		{
			std::string entry = dir_entry.path().filename().string();

			if (dir_entry.is_directory())
			{
				ImGui::TableNextColumn();

				ImGui::ImageButton((void*)AssetManager::Get<Texture2D>(m_DirectoryTextureHandle).GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth), ImVec2(0, 0), ImVec2(1, 1), -1, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
				
				//ImGui::ImageButton((void*)AssetManager::GetAsset<Texture2D>(aex).GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth), ImVec2(0, 0), ImVec2(1, 1), -1, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				{
					currentDirectory /= entry;
					m_SelectedDirectory = entry;
				}

				ImGui::SameLine();
				ImGui::NewLine();

				RenderThumbnailText(entry, ImGui::IsItemHovered());
			}
		}


		for (auto const& dir_entry : std::filesystem::directory_iterator{ currentDirectory })
		{
			std::string entry = dir_entry.path().filename().string();
			
			std::string entryPath = dir_entry.path().string();
			std::string entryStem = dir_entry.path().filename().stem().string();
			std::string extension = dir_entry.path().filename().extension().string();

			if (extension == ".jpg" || extension == ".png")
			{
				ImGui::TableNextColumn();

				AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(entryPath);
				RenderAssetThumbnail<Texture2D>(textureHandle, textureHandle, textureHandle, entry, m_ThumbnailWidth);
			}
			else if (extension == ".Relentless")
			{
				ImGui::TableNextColumn();

				ImGui::ImageButton((ImTextureID)AssetManager::Get<Texture2D>(m_SceneTextureHandle).GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));

				if (ImGui::BeginDragDropSource())
				{
					const char* path = entryPath.c_str();
					ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
					ImGui::BeginTooltip();
					ImGui::Image((ImTextureID)AssetManager::Get<Texture2D>(m_SceneTextureHandle).GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth));
					ImGui::EndTooltip();
					ImGui::PopStyleVar(1);
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_SCENE", path, strlen(path) + 1, ImGuiCond_::ImGuiCond_Once);
					ImGui::EndDragDropSource();
				}

				ImGui::SameLine();
				ImGui::NewLine();


				
				RenderThumbnailText(entry, ImGui::IsItemHovered());
			}
			else if (extension == ".rmat")
			{
				ImGui::TableNextColumn();

				RLS_ASSERT(false, "TODO!!!");
				//const AssetHandle materialHandle = AssetManager::LoadFromFile<Material>(entryPath);
				//RenderAssetThumbnail<Material>(m_MaterialTextureHandle, m_MaterialTextureHandle, materialHandle, AssetManager::Get<Material>(materialHandle).GetName(), m_ThumbnailWidth);
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
					RLS_ASSERT(false, "STOP");
					std::filesystem::copy(filePath, currentDirectory, std::filesystem::copy_options::overwrite_existing);
					std::filesystem::path copiedTexturePath = currentDirectory / filePath.filename();
					//UUID uuid = TextureSerializer::SerializeDefault(copiedTexturePath.string());
					//AssetManager::MapGUIDToFilepath(uuid, copiedTexturePath.string());
					AssetHandle textureHandle = AssetManager::LoadFromFile<Texture2D>(copiedTexturePath.string());
					AssetRegistry::MapAssetToFilepath({ textureHandle.Uuid, AssetType::Texture2D }, copiedTexturePath.string());
				}
			}
			if (ImGui::MenuItem("New Material"))
			{
				static uint32_t createdMaterialCounter{ 0u };
				createdMaterialCounter = 0u;
				std::string name = "New Material[" + std::to_string(++createdMaterialCounter) + "]";
				std::filesystem::path path = currentDirectory / name;
				path.append(".rasset");
				while (AssetManager::IsLoaded(path.string()))
				{
					name = "New Material[" + std::to_string(++createdMaterialCounter) + "]";
					path = currentDirectory / name;
					path.append(".rasset");
				}

				//m_AssetToName = AssetManager::LoadFromFile<Material>((currentDirectory / name).string());
				m_AssetToName = AssetManager::CreateNew<Material>();
				Material& newMaterial = AssetManager::Get<Material>(m_AssetToName);
				newMaterial.SetName(name);
				//MaterialSerializer::Serialize(m_AssetToName, path.string());
				Serializer::Serialize<Material>(m_AssetToName, path.string());
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

	//Function assumes correct placement relative other imgui calls beforehand!
	//TODO: Change to accomodate more assets than just materials!!
	void ContentBrowserPanel::EditThumbnailText(const AssetHandle& handle) noexcept
	{
		constexpr float textFieldWidthPadding = 8.0f;
		ImGui::PushItemWidth(m_ThumbnailWidth + textFieldWidthPadding);

		static char newName[64] = "";

		if (m_FirstTimeEditingThumbnail)
		{
			std::string materialName = AssetManager::Get<Material>(handle).GetName();
			std::size_t lengthToCopy = std::min(materialName.size(), sizeof(newName) - 1);
			std::memcpy(newName, materialName.c_str(), lengthToCopy);
			newName[lengthToCopy] = '\0';
		}

		ImGui::SetKeyboardFocusHere();

		m_FirstTimeEditingThumbnail = false;
		if (ImGui::InputText("##NameAssetInput", newName, 64, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string nameAsString = std::string(newName);
			std::filesystem::path fullPath = (currentDirectory / nameAsString).concat(".rmat");

			if (!nameAsString.empty() && !std::filesystem::exists(fullPath))
			{
				const std::string materialNametoReplace = AssetManager::Get<Material>(handle).GetName();
				const std::string materialFilename = materialNametoReplace + ".rasset";
				AssetManager::Get<Material>(handle).SetName(newName);

				std::filesystem::path fullPathToSave = currentDirectory;
				fullPathToSave.append(std::string(newName) + ".rasset");
				std::filesystem::path fullPathToDelete = currentDirectory;
				fullPathToDelete.append(materialFilename);

				Serializer::Serialize<Material>(m_AssetToName, fullPathToSave.string());
				std::filesystem::remove(fullPathToDelete.string());
				memset(newName, 0, 64);

				AssetManager::OnFileMoved(m_AssetToName, fullPathToSave.string());
				AssetRegistry::RemoveUUIDToFilepathMap(m_AssetToName.Uuid, fullPathToDelete.string());
				AssetRegistry::MapAssetToFilepath({ m_AssetToName.Uuid, AssetType::Material }, fullPathToSave.string());
				m_AssetToName = NULL_HANDLE;
			}
			m_FirstTimeEditingThumbnail = true;
		}
		ImGui::PopItemWidth();
	}
}