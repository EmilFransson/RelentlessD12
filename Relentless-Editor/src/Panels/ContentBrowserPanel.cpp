#include "ContentBrowserPanel.h"
namespace Relentless
{
	static std::filesystem::path currentDirectory = EDITOR_ASSET_DIRECTORY;

	ContentBrowserPanel::ContentBrowserPanel() noexcept
	{
		m_DirectoryTextureHandle = Serializer::Deserialize<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\directoryicon.rasset");
		m_OpenDirectoryTextureHandle = Serializer::Deserialize<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\directoryiconopen2.rasset");
		m_SceneTextureHandle = Serializer::Deserialize<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\SceneThumbnail.rasset");
		m_MaterialTextureHandle = Serializer::Deserialize<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\MaterialThumbnail.rasset");
		m_MeshTextureHandle = Serializer::Deserialize<Texture2D>(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\meshthumbnail.rasset");

		currentDirectory = currentDirectory.string().substr(0u, currentDirectory.string().size() - 1);
	}

	void ContentBrowserPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;

		PROFILE_FUNC;

		m_DisplayedEntries = 0u;

		RenderMenuBar();

		const ImVec2 windowSize = ImGui::GetCurrentWindow()->Size;
		const float availableWidth = windowSize.x;
		const float splitterThickness = 4.0f;

		// Calculate pane widths based on the current window size
		const float leftPaneWidth = (availableWidth - splitterThickness) * 0.5f + m_DragAmount;
		// Ensure the calculation does not allow the panes to exceed the window's width
		const float rightPaneWidth = availableWidth - leftPaneWidth - splitterThickness - 45.0f;
		

		ImGui::BeginChild("ContentBrowserLeftPanel", ImVec2(leftPaneWidth, 0));

		//RenderDirectoryHierarchySearchBar();

		RenderDirectoryHierarchy();
		ImGui::EndChild();

		ImGui::SameLine();

		// Splitter behavior
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - splitterThickness);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(22.0f / 255, 22.0f / 255, 22.0f / 255, 1.0f));
		ImGui::Button("##Splitter", ImVec2(splitterThickness, -1));
		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);
		if (ImGui::IsItemActive()) 
			m_DragAmount += ImGui::GetIO().MouseDelta.x;

		ImGui::PopStyleColor();
		ImGui::SameLine();

		ImGui::BeginChild("ContentBrowserRightPanel", ImVec2(rightPaneWidth, 0));

		RenderAssetHierarchyOverview();
		RenderAssetSearchBox();
		RenderAssetThumbNails();

		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 p_min = ImGui::GetCursorScreenPos(); // Top-left corner of the rectangle
		p_min.y -= 14.0f;
		ImVec2 p_max = ImVec2(p_min.x + ImGui::GetContentRegionAvail().x, p_min.y + 10.0f); // Bottom-right corner, specify your desired width and height

		// Define colors for each corner: top-left, top-right, bottom-left, bottom-right
		ImVec4 col_top_left = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];// Dark color
		ImVec4 col_top_right = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];// Dark color
		ImU32 col_bottom_left = IM_COL32(20, 20, 20, 255); // Less dark color
		ImU32 col_bottom_right = IM_COL32(20, 20, 20, 255); // Less dark color

		draw_list->AddRectFilledMultiColor(p_min, p_max, ImGui::ColorConvertFloat4ToU32(col_top_left), ImGui::ColorConvertFloat4ToU32(col_top_right), col_bottom_right, col_bottom_left);
		
		const std::string entryDisplayText = std::format("{} {}", m_DisplayedEntries, (m_DisplayedEntries > 1 || m_DisplayedEntries == 0) ? "items" : "item");
		ImGui::Text(entryDisplayText.c_str());
		
		RenderPopUpOptions();

		ImGui::EndChild();

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
		{
			DeselectAllContentBrowserEntries();
			m_OnAssetSelectedCallback(NULL_HANDLE, InspectedAssetType::NONE);
		}

		ImGui::End();
	}

	void ContentBrowserPanel::SetOnAssetSelectedCallback(std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> callback) noexcept
	{
		m_OnAssetSelectedCallback = callback;
	}

	void ContentBrowserPanel::RenderDirectoryHierarchy() noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;
		
		//Background: TODO: CHANGE TO JUST A CALL FOR FULL CHILD WINDOW.
		const float remainingWidth = ImGui::GetContentRegionMax().x;
		ImVec2 directoryHierarchyMinPoint = ImGui::GetCursorScreenPos();

		const float maxPanelYPosition = ImGui::GetContentRegionMax().y;

		const ImVec2 directoryHierarchyMaxPoint = ImVec2(directoryHierarchyMinPoint.x + remainingWidth, directoryHierarchyMinPoint.y + maxPanelYPosition);

		constexpr const ImU32 rectColor = IM_COL32(26.0f, 26.0f, 26.0f, 255.0f);
		pDrawList->AddRectFilled(directoryHierarchyMinPoint, directoryHierarchyMaxPoint, rectColor);
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (ImGui::IsWindowHovered())
				m_DirectoryHierarchyFocused = true;
			else
				m_DirectoryHierarchyFocused = false;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 6.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(46.0f, 46.0f, 46.0f, 255.0f));

		ImGuiTreeNodeFlags sceneNodeflags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;
		sceneNodeflags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
		ImGui::PushFont(boldFont);
		ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(44.0f, 50.0f, 58.0f, 255.0f));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(44.0f, 50.0f, 58.0f, 255.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
		ImVec4 originalBorderColor = ImGui::GetStyle().Colors[ImGuiCol_Border];
		ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); 
		
		const ImVec2 positionBeforeNode = ImGui::GetCursorPos();
		const bool opened = ImGui::TreeNodeEx("Content", sceneNodeflags, "    Content");
		const ImVec2 positionAfterNode = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(positionBeforeNode.x + 25.0f, positionBeforeNode.y + 4.0f));
		
		if (opened)
		{
			const Texture& openDirectoryTexture = AssetManager::Get<Texture2D>(m_OpenDirectoryTextureHandle);
			ImGui::Image((ImTextureID)openDirectoryTexture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.78f, 0.38f, 1.0f));
		}
		else
		{
			const Texture& closedDirectoryTexture = AssetManager::Get<Texture2D>(m_DirectoryTextureHandle);
			ImGui::Image((ImTextureID)closedDirectoryTexture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.78f, 0.38f, 1.0f));
		}
		
		ImGui::SetCursorPos(positionAfterNode);

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
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

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(1);
		ImGui::GetStyle().Colors[ImGuiCol_Border] = originalBorderColor;
	}

	void ContentBrowserPanel::DrawDirectoryNode(const std::filesystem::directory_entry& directoryEntry) noexcept
	{
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(30, 120, 255, 200));

		const bool isSelected = IsDirectorySelectedInHierarchy(directoryEntry.path().string());
		const bool isAncestorToAnySelectedDirectory = IsAncestorDirectoryToAnySelectedDirectory(directoryEntry.path().string());
		if (isSelected)
		{
			ImGui::PushStyleColor(ImGuiCol_Header, m_DirectoryHierarchyFocused ? IM_COL32(30, 120, 255, 200) : IM_COL32(64.0f, 87.0f, 111.0f, 255.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, m_DirectoryHierarchyFocused ? IM_COL32(30, 120, 255, 200) : IM_COL32(64.0f, 87.0f, 111.0f, 255.0f));
		}
		else if (isAncestorToAnySelectedDirectory)
		{
			ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(44.0f, 50.0f, 58.0f, 255.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(44.0f, 50.0f, 58.0f, 255.0f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(26.0f, 26.0f, 26.0f, 255.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(36.0f, 36.0f, 36.0f, 255.0f));
		}

		ImGuiTreeNodeFlags directoryNodeflags = (isSelected || isAncestorToAnySelectedDirectory) ? ImGuiTreeNodeFlags_Selected : 0;
		directoryNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;
		directoryNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Framed;

		ImGuiStyle* pStyle = &ImGui::GetStyle();
		pStyle->Alpha = isSelected || isAncestorToAnySelectedDirectory ? 1.0f : 0.5f;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		const std::string& directoryEntryName = directoryEntry.path().filename().string();

		const std::string directoryNodeName = "    " + directoryEntryName;
		const ImVec2 positionBeforeNode = ImGui::GetCursorPos();
		const bool opened = ImGui::TreeNodeEx(directoryEntry.path().string().c_str(), directoryNodeflags, directoryNodeName.c_str());
		const ImVec2 positionAfterNode = ImGui::GetCursorPos();
		
		ImGui::PopStyleVar();
		
		if (ImGui::IsItemClicked())
		{
			currentDirectory = directoryEntry;
			const bool lCtrlPressed = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
			if (!lCtrlPressed)
			{
				if (!isSelected)
					DeselectAllContentBrowserEntries();

				DeselectAllHierarchyDirectories();
			}

			if (!IsDirectorySelectedInHierarchy(directoryEntry.path().string()))
				SelectHiearchyDirectory(directoryEntry.path().string());
			else
			{
				if (GetSelectedHierarchyDirectoriesCount() > 1)
					DeselectHiearchyDirectory(directoryEntry.path().string());
			}
		}

		ImGui::PopStyleColor(3);

		pStyle->Alpha = 1.0f;

		ImGui::SetCursorPos(ImVec2(positionBeforeNode.x + 25.0f, positionBeforeNode.y + 4.0f));

		if (opened)
		{
			const Texture& openDirectoryTexture = AssetManager::Get<Texture2D>(m_OpenDirectoryTextureHandle);
			ImGui::Image((ImTextureID)openDirectoryTexture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.78f, 0.38f, 1.0f));
		}
		else
		{
			const Texture& closedDirectoryTexture = AssetManager::Get<Texture2D>(m_DirectoryTextureHandle);
			ImGui::Image((ImTextureID)closedDirectoryTexture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.58f, 0.58f, 1.0f));
		}
		ImGui::SetCursorPos(positionAfterNode);

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
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);  // Black color
			ImGui::Separator();
	}

	void ContentBrowserPanel::RenderDirectoryHierarchySearchBar() noexcept
	{
		UI::SearchBar("DirectoryHierarchySearchBar", "Search Paths...");
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
		m_ContentFilter = UI::SearchBar("AssetSearchBar", ConstructAssetBrowserHintString().c_str(), true, 600.0f);
	}

	void ContentBrowserPanel::RenderAssetThumbNails() noexcept
	{
		ImGui::BeginChild("TableContainer", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y - 40.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
		
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
			DeselectAllContentBrowserEntries();

		static const ImVec2 padding = ImVec2(20.0f, 10.0f);
		const float cellSize = m_ThumbnailWidth + padding.x;

		const float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);

		if (columnCount < 1)
			columnCount = 1	;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, padding.y)); // Adjust cell padding
		
		ImGui::BeginTable("##MyTable", columnCount);
		ImGui::TableNextColumn();

		//Directories should always display first:
		for (auto& directoryPath : m_SelectedHierarchyDirectories)
		{
			if (directoryPath.empty())
				break;

			for (auto const& directoryEntry : std::filesystem::directory_iterator{ directoryPath })
			{
				if (directoryEntry.is_directory())
				{
					DrawDirectoryThumbnail(directoryEntry.path());
					m_DisplayedEntries++;
					
					ImGui::TableNextColumn();
				}
			}
		}

		for (auto& directoryPath : m_SelectedHierarchyDirectories)
		{
			if (directoryPath.empty())
				break;

			for (auto const& dir_entry : std::filesystem::directory_iterator{ directoryPath })
			{
				if (dir_entry.is_directory())
					continue;

				std::string extension = dir_entry.path().filename().extension().string();

				if (extension != ".rasset")
					continue;

				std::string entry = dir_entry.path().filename().string();
				std::string entryPath = dir_entry.path().string();
				std::string entryStem = dir_entry.path().filename().stem().string();

				const bool IsUsingFilter = !m_ContentFilter.empty();
				if (IsUsingFilter)
				{
					std::string entryStemToLower = dir_entry.path().filename().string();
					std::string contentFilterToLower = m_ContentFilter;

					std::transform(entryStemToLower.begin(), entryStemToLower.end(), entryStemToLower.begin(), ::tolower);
					std::transform(contentFilterToLower.begin(), contentFilterToLower.end(), contentFilterToLower.begin(), ::tolower);

					if (entryStemToLower.find(contentFilterToLower) == std::string::npos)
						continue;
				}

				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				const ImVec2 vMin(ImGui::GetCursorScreenPos());
				const ImVec2 vMax(vMin.x + m_ThumbnailWidth, vMin.y + m_ThumbnailHeight);

				const ImVec2 vMinDropShadow(ImGui::GetCursorScreenPos().x + 6.0f, vMin.y + 6.0f);
				const ImVec2 vMaxDropShadow(vMinDropShadow.x + m_ThumbnailWidth, vMinDropShadow.y + m_ThumbnailHeight);
				constexpr const ImU32 black = IM_COL32(0, 0, 0, 128);
				pDrawList->AddRectFilled(vMinDropShadow, vMaxDropShadow, black, 7.0f);


				const ImVec2 mousePos = ImGui::GetMousePos();

				bool isHovered = false;
				if (mousePos.x >= vMin.x && mousePos.x <= vMax.x && mousePos.y >= vMin.y && mousePos.y <= vMax.y)
					isHovered = true;

				const bool isSelected = IsEntrySelected(entryPath);

				const ImU32 unrealGrey = isSelected ? IM_COL32(30, 120, 255, 200) : isHovered ? IM_COL32(80, 80, 80, 255) : IM_COL32(60, 60, 60, 255);

				const ImVec2 rectMin = ImVec2(vMin.x, vMin.y + m_ThumbnailWidth);
				const ImVec2 rectMax = ImVec2(vMax.x, vMax.y);

				ImVec4 bgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
				pDrawList->AddRectFilled(vMin, vMax, IM_COL32(bgColor.x, bgColor.y, bgColor.z, bgColor.w), 5.0f);
				pDrawList->AddRectFilled(rectMin, rectMax, unrealGrey, 5.0f);

				if (isHovered || isSelected)
				{
					if (isHovered)
						ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Hand);

					const ImVec2 topLineMinPosition = ImVec2(vMin.x, vMin.y - 2.0f);
					const ImVec2 topLineMaxPosition = ImVec2(vMax.x, vMin.y - 2.0f);
					const ImVec2 leftLineMinPosition = ImVec2(vMin.x, vMin.y - 2.0f + m_ThumbnailWidth);
					const ImVec2 rightLineMinPosition = ImVec2(vMax.x, vMin.y - 2.0f + m_ThumbnailWidth);
					pDrawList->AddLine(topLineMinPosition, topLineMaxPosition, unrealGrey, 0.5f);
					pDrawList->AddLine(topLineMinPosition, leftLineMinPosition, unrealGrey, 0.5f);
					pDrawList->AddLine(topLineMaxPosition, rightLineMinPosition, unrealGrey, 0.5f);
				}

				const ImVec2 vMinLine(vMin.x + 0.8f, vMin.y + m_ThumbnailWidth);
				const ImVec2 vMaxLine(vMinLine.x + m_ThumbnailWidth - 1.6f, vMin.y + m_ThumbnailWidth);

				const AssetType assetType = AssetRegistry::GetMetaData(entryPath).AssetType;
				ImU32 lineColor;
				if (assetType == AssetType::Texture2D)
					lineColor = IM_COL32(180, 0, 0, 255);
				else if (assetType == AssetType::Mesh)
					lineColor = IM_COL32(54, 214, 198, 255);
				else if (assetType == AssetType::Material)
					lineColor = IM_COL32(0, 180, 0, 255);


				pDrawList->AddLine(vMinLine, vMaxLine, lineColor, 3.0f);

				const ImVec2 textPosition(vMin.x + 5.0f, vMax.y - 25.0f);

				const ImVec2 windowPos = ImGui::GetWindowPos();
				const ImVec2 relativePos = ImVec2(vMin.x - windowPos.x + 3.0f, vMin.y - windowPos.y + ImGui::GetScrollY());
				ImGui::SetCursorPos(relativePos);
				const ImVec2 assetNameTextPosition = ImVec2(vMinLine.x + 7.0f, vMinLine.y + 3.0f);
				const float originalBorderSize = ImGui::GetStyle().FrameBorderSize;
				ImGui::GetStyle().FrameBorderSize = 0.0f;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				if (assetType == AssetType::Texture2D)
				{
					const AssetHandle textureHandle = Serializer::Deserialize<Texture2D>(entryPath);
					const Texture2D& texture = AssetManager::Get<Texture2D>(textureHandle);
					const std::string name = AssetManager::Get<Texture2D>(textureHandle).GetName();
					const std::string displayName = std::filesystem::path(name).filename().string();


					ImGui::ImageButton((ImTextureID)texture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth - 5.0f, m_ThumbnailWidth));
					ImGui::SetItemAllowOverlap();

					
					ImGui::PopStyleVar();
					ImGui::GetStyle().FrameBorderSize = originalBorderSize;

					constexpr const char* text = "Texture";
					pDrawList->AddText(textPosition, IM_COL32(255, 255, 255, 140), text);

					pDrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), assetNameTextPosition, IM_COL32(255, 255, 255, 200), displayName.c_str(), (const char*)0, m_ThumbnailWidth - 14.0f);
				}
				else if (assetType == AssetType::Material)
				{
					const Texture2D& texture = AssetManager::Get<Texture2D>(m_MaterialTextureHandle);
					const std::string displayName = std::filesystem::path(entry).filename().string();

					ImGui::ImageButton((ImTextureID)texture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth - 5.0f, m_ThumbnailWidth));
					ImGui::SetItemAllowOverlap();

					
					ImGui::PopStyleVar();

					constexpr const char* text = "Material";
					pDrawList->AddText(textPosition, IM_COL32(255, 255, 255, 140), text);

					pDrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), assetNameTextPosition, IM_COL32(255, 255, 255, 200), displayName.c_str(), (const char*)0, m_ThumbnailWidth - 14.0f);
				}
				else if (assetType == AssetType::Mesh)
				{
					const Texture2D& texture = AssetManager::Get<Texture2D>(m_MeshTextureHandle);
					const std::string displayName = std::filesystem::path(entry).filename().string();

					ImGui::ImageButton((ImTextureID)texture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth - 5.0f, m_ThumbnailWidth));
					ImGui::SetItemAllowOverlap();
					
					ImGui::PopStyleVar();

					constexpr const char* text = "Mesh";
					pDrawList->AddText(textPosition, IM_COL32(255, 255, 255, 140), text);

					pDrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), assetNameTextPosition, IM_COL32(255, 255, 255, 200), displayName.c_str(), (const char*)0, m_ThumbnailWidth - 14.0f);
				}

				ImGui::SetCursorPos(relativePos);
				ImGui::InvisibleButton(entry.c_str(), ImVec2(vMax.x - vMin.x, vMax.y - vMin.y));
				if (ImGui::IsItemClicked())
				{
					const bool lCtrlPressed = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
					const bool isSelected = IsEntrySelected(entryPath);
					if (lCtrlPressed && isSelected)
					{
						DeselectEntry(entryPath);
					}
					else if (lCtrlPressed)
					{
						SelectEntry(entryPath);
					}
					else
					{
						DeselectAllContentBrowserEntries();
						SelectEntry(entryPath);
					}
				}
				m_DisplayedEntries++;

				ImGui::TableNextColumn();
			}
		}
		ImGui::EndTable();
		ImGui::PopStyleVar();

		ImGui::EndChild();
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
					std::filesystem::copy(filePath, currentDirectory, std::filesystem::copy_options::overwrite_existing);
					std::filesystem::path copiedTexturePath = currentDirectory / filePath.filename();
					AssetManager::LoadFromFile<Texture2D>(copiedTexturePath.string());
				}
			}
			if (ImGui::MenuItem("New Material"))
			{
				static uint32_t createdMaterialCounter{ 0u };
				createdMaterialCounter = 0u;
				std::string name = "New Material[" + std::to_string(++createdMaterialCounter) + "]";
				std::filesystem::path path = currentDirectory / name;
				path += ".rasset";
				while (AssetManager::IsLoaded(path.string()))
				{
					name = "New Material[" + std::to_string(++createdMaterialCounter) + "]";
					path = currentDirectory / name;
					path.append(".rasset");
				}

				m_AssetToName = AssetManager::CreateNew<Material>();
				Material& newMaterial = AssetManager::Get<Material>(m_AssetToName);
				newMaterial.SetName(name);
				Serializer::Serialize<Material>(m_AssetToName, path.string());
				
				AssetMetaData amd;
				amd.AssetType = AssetType::Material;
				amd.Uuid = m_AssetToName.Uuid;
				AssetRegistry::MapAssetToFilepath(amd, path.string());
			}
			ImGui::EndPopup();
		}

		if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
			m_SelectedAsset = NULL_UUID;
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

	void ContentBrowserPanel::DrawDirectoryThumbnail(const std::filesystem::path directoryPath) noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		const std::string directoryName = directoryPath.filename().string();

		const ImVec2 thumbnailMinPoint(ImGui::GetCursorScreenPos());
		const ImVec2 thumbnailMaxPoint(thumbnailMinPoint.x + m_ThumbnailWidth, thumbnailMinPoint.y + m_ThumbnailHeight);

		ImGui::InvisibleButton
		(
			directoryPath.string().c_str(), 
			ImVec2(thumbnailMaxPoint.x - thumbnailMinPoint.x, thumbnailMaxPoint.y - thumbnailMinPoint.y)
		);
		
		const bool isClicked = ImGui::IsItemClicked();
		const bool isHovered = ImGui::IsItemHovered();
		const bool isLeftMouseDoubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left);
		const bool isSelected = IsEntrySelected(directoryPath.string());

		if (isHovered && isLeftMouseDoubleClicked)
		{
			currentDirectory /= directoryName;
			DeselectAllHierarchyDirectories();
			DeselectAllContentBrowserEntries();
			SelectHiearchyDirectory(directoryPath.string());
			SelectEntry(directoryPath.string());
		}
		else if (isClicked)
		{
			const bool lCtrlDown = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
			if (isSelected && lCtrlDown)
			{
				DeselectEntry(directoryPath.string());
			}
			else
			{
				if (!isSelected)
					SelectEntry(directoryPath.string());
			}
		}

		if (isHovered || isSelected)
		{
			const ImVec2 vMinDropShadow(thumbnailMinPoint.x + 6.0f, thumbnailMinPoint.y + 6.0f);
			const ImVec2 vMaxDropShadow(vMinDropShadow.x + m_ThumbnailWidth, vMinDropShadow.y + m_ThumbnailHeight);
			constexpr const ImU32 black = IM_COL32(0, 0, 0, 128);
			pDrawList->AddRectFilled(vMinDropShadow, vMaxDropShadow, black, 7.0f);
		}
		if (isHovered)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Hand);
		}
		
		const ImU32 thumbnailColor = isSelected ? IM_COL32(0, 112, 224, 255) : isHovered ? IM_COL32(60, 60, 60, 255) : IM_COL32(0, 0, 0, 0);

		pDrawList->AddRectFilled(thumbnailMinPoint, thumbnailMaxPoint, thumbnailColor, 5.0f);

		const ImVec2 vMinLine(thumbnailMinPoint.x + 0.8f, thumbnailMinPoint.y + m_ThumbnailWidth);

		if (isSelected)
		{
			const ImVec2 imageDividerMinPoint = ImVec2(thumbnailMinPoint.x + 2.0f, thumbnailMinPoint.y + 2.0f);
			const ImVec2 imageDividerMaxPoint = ImVec2(thumbnailMaxPoint.x - 2.0f, thumbnailMinPoint.y + m_ThumbnailWidth);

			pDrawList->AddRectFilled(imageDividerMinPoint, imageDividerMaxPoint, IM_COL32(60, 60, 60, 255));
		}

		const ImVec2 windowPos = ImGui::GetWindowPos();
		const ImVec2 relativePos = ImVec2(thumbnailMinPoint.x - windowPos.x, thumbnailMinPoint.y - windowPos.y + ImGui::GetScrollY());
		ImGui::SetCursorPos(relativePos);

		const float textLength = ImGui::CalcTextSize(directoryName.c_str()).x;
		const ImVec2 assetNameTextPosition = ImVec2((thumbnailMinPoint.x + (m_ThumbnailWidth / 2.0f)) - (textLength / 2.0f), vMinLine.y + 3.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

		ImGui::ImageButton((void*)AssetManager::Get<Texture2D>(m_DirectoryTextureHandle).GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(0.58f, 0.58f, 0.58f, 1.0f));

		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar();

		pDrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), assetNameTextPosition, IM_COL32(255, 255, 255, 200), directoryName.c_str(), (const char*)0, m_ThumbnailWidth - 14.0f);
	}

	bool ContentBrowserPanel::IsEntrySelected(const std::string& entryPath) const noexcept
	{
		return std::find(m_SelectedEntries.begin(), m_SelectedEntries.end(), entryPath) != m_SelectedEntries.end();
	}

	void ContentBrowserPanel::SelectEntry(const std::string& entryPath) noexcept
	{
		RLS_ASSERT(std::find(m_SelectedEntries.begin(), m_SelectedEntries.end(), entryPath) == m_SelectedEntries.end(), "[ContentBrowserPanel]: Entry already selected.");
		m_SelectedEntries.push_back(entryPath);
	}

	void ContentBrowserPanel::DeselectEntry(const std::string& entryPath) noexcept
	{
		auto it = std::find(m_SelectedEntries.begin(), m_SelectedEntries.end(), entryPath);
		RLS_ASSERT(it != m_SelectedEntries.end(), "[ContentBrowserPanel]: Entry to deselect is not already selected");
		m_SelectedEntries.erase(it);
	}

	bool ContentBrowserPanel::IsDirectorySelectedInHierarchy(const std::string& directoryPath) const
	{
		RLS_ASSERT(std::filesystem::is_directory(directoryPath), "[ContentBrowserPanel]: Entry is not of type directory");
		return std::find(m_SelectedHierarchyDirectories.begin(), m_SelectedHierarchyDirectories.end(), directoryPath) != m_SelectedHierarchyDirectories.end();
	}

	void ContentBrowserPanel::SelectHiearchyDirectory(const std::string& directoryPath) noexcept
	{
		RLS_ASSERT(std::find(m_SelectedHierarchyDirectories.begin(), m_SelectedHierarchyDirectories.end(), directoryPath) == m_SelectedHierarchyDirectories.end(), "[ContentBrowserPanel]: Directory already selected.");
		m_SelectedHierarchyDirectories.push_back(directoryPath);
	}

	void ContentBrowserPanel::DeselectHiearchyDirectory(const std::string& directoryPath) noexcept
	{
		RLS_ASSERT(std::filesystem::is_directory(directoryPath), "[ContentBrowserPanel]: Entry is not of type directory");

		auto it = std::find(m_SelectedHierarchyDirectories.begin(), m_SelectedHierarchyDirectories.end(), directoryPath);
		RLS_ASSERT(it != m_SelectedHierarchyDirectories.end(), "[ContentBrowserPanel]: Directory to deselect is not already selected");
		m_SelectedHierarchyDirectories.erase(it);
	}

	void ContentBrowserPanel::DeselectAllHierarchyDirectories() noexcept
	{
		m_SelectedHierarchyDirectories.clear();
	}

	void ContentBrowserPanel::DeselectAllContentBrowserEntries() noexcept
	{
		m_SelectedEntries.clear();
	}

	uint32_t ContentBrowserPanel::GetSelectedHierarchyDirectoriesCount() const
	{
		return m_SelectedHierarchyDirectories.size();
	}

	bool ContentBrowserPanel::IsAncestorDirectoryToAnySelectedDirectory(const std::filesystem::path& directoryPath) const
	{
		for (auto& selectedDirectory : m_SelectedHierarchyDirectories)
		{
			std::filesystem::path possibleChildDirectoryPath = std::filesystem::path(selectedDirectory);
			auto [parentIt, childIt] = std::mismatch(directoryPath.begin(), directoryPath.end(), possibleChildDirectoryPath.begin(), possibleChildDirectoryPath.end());
		
			if (parentIt == directoryPath.end() && childIt != possibleChildDirectoryPath.end())
				return true;
		}

		return false;
	}

	std::string ContentBrowserPanel::ConstructAssetBrowserHintString() const noexcept
	{
		if (m_SelectedHierarchyDirectories.empty())
			return "Search Content";

		std::string constructedHint = "Search " + std::filesystem::path(m_SelectedHierarchyDirectories[0]).filename().string();

		for (uint32_t i{ 1u }; i < m_SelectedHierarchyDirectories.size(); ++i)
		{
			constructedHint += ", ";
			constructedHint += std::filesystem::path(m_SelectedHierarchyDirectories[i]).filename().string();
		}
		
		return std::move(constructedHint);
	}
}