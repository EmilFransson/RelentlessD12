#include "ContentBrowserPanel.h"
namespace Relentless
{
	static std::string editorAssetDirectory = EDITOR_ASSET_DIRECTORY;
	static std::filesystem::path currentDirectory = "";

	ContentBrowserPanel::ContentBrowserPanel() noexcept
	{
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\directoryicon.rasset", m_DirectoryTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\directoryiconopen2.rasset", m_OpenDirectoryTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\SceneThumbnail.rasset", m_SceneTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\MaterialThumbnail.rasset", m_MaterialTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\MeshThumbnail.rasset", m_MeshTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\importicon.rasset", m_ImportIconTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\filtericon.rasset", m_FilterIconTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\arrowdownicon.rasset", m_DownArrowIconTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\undoicon.rasset", m_UndoArrowIconTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\plusicon.rasset", m_PlusIconTextureHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\rightarrowicon.rasset", m_RightArrowIconTextureHandle), "Core engine icon missing.");

		editorAssetDirectory = editorAssetDirectory.substr(0u, editorAssetDirectory.size() - 1);
		m_SelectedHierarchyDirectories.push_back(editorAssetDirectory);
		currentDirectory = editorAssetDirectory;
	}

	void ContentBrowserPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;

		PROFILE_FUNC;

		m_DisplayedEntries = 0u;

		if (!ImGui::Begin("Content Browser", nullptr))
		{
			ImGui::End();
			return;
		}

		RenderMenuBar();

		const float availableWidth = ImGui::GetCurrentWindow()->Size.x;
		constexpr const float splitterThickness = 4.0f;
		const float leftChildWidth = (availableWidth - splitterThickness) * 0.5f + m_DragAmount;
		const float rightChildWidth = availableWidth - leftChildWidth - splitterThickness - 45.0f;

		RenderLeftChildWindow(leftChildWidth);
		RenderChildWindowsDraggableSplitter(splitterThickness);
		RenderRightChildWindow(rightChildWidth);

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && !Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
		{
			DeselectAllContentBrowserEntries();
			m_OnAssetSelectedCallback(NULL_HANDLE, InspectedAssetType::NONE);
		}

		ImGui::End();
	}

	void ContentBrowserPanel::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::KeyPressedEvent:
		{
			const RLS_KEY key = EVENT(KeyPressedEvent).key;
			switch (key)
			{
			case RLS_KEY::A:
				if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl) && m_AssetHierarchyFocused)
				{
					SelectAllEntriesInDirectories(m_SelectedHierarchyDirectories);
				}
				break;
			}
			break;
		}
		}
	}

	void ContentBrowserPanel::SetOnAssetSelectedCallback(std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> callback) noexcept
	{
		m_OnAssetSelectedCallback = callback;
	}

	const std::vector<std::string>& ContentBrowserPanel::GetSelectedEntries() const noexcept
	{
		return m_SelectedEntries;
	}

	void ContentBrowserPanel::RenderDirectoryHierarchy() noexcept
	{
		ImGuiTreeNodeFlags sceneNodeflags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;
		sceneNodeflags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_OpenOnArrow;

		auto boldFont = ImGui::GetIO().Fonts->Fonts[OPENSANS_BOLD_18];
		ImGui::PushFont(boldFont);

		const bool isSelected = IsDirectorySelectedInHierarchy(editorAssetDirectory);
		const bool isAncestorToAnySelectedDirectory = IsAncestorDirectoryToAnySelectedDirectory(editorAssetDirectory);
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

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f));
		
		const ImVec4 originalBorderColor = ImGui::GetStyle().Colors[ImGuiCol_Border];
		ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f); 
		
		const ImVec2 positionBeforeNode = ImGui::GetCursorPos();
		const bool opened = ImGui::TreeNodeEx(editorAssetDirectory.c_str(), sceneNodeflags, "   Assets");

		if (ImGui::IsItemClicked())
		{
			currentDirectory = editorAssetDirectory;
			const bool lCtrlPressed = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
			if (!lCtrlPressed)
			{
				if (!isSelected)
				{
					DeselectAllContentBrowserEntries();
				}

				DeselectAllHierarchyDirectories();
				SelectHiearchyDirectory(editorAssetDirectory);
			}
			else
			{
				if (!isSelected)
				{
					SelectHiearchyDirectory(editorAssetDirectory);
				}
				else
				{
					DeselectHiearchyDirectory(editorAssetDirectory);
				}
			}
		}


		const ImVec2 positionAfterNode = ImGui::GetCursorPos();
		
		ImGui::SetCursorPos(ImVec2(positionBeforeNode.x + 30.0f, positionBeforeNode.y + 6.0f));
		if (opened)
		{
			const std::shared_ptr<Texture> openDirectoryTexture = AssetManager::Get<Texture2D>(m_OpenDirectoryTextureHandle);
			ImGui::Image((ImTextureID)openDirectoryTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.78f, 0.38f, 1.0f));
		}
		else
		{
			const std::shared_ptr<Texture> closedDirectoryTexture = AssetManager::Get<Texture2D>(m_DirectoryTextureHandle);
			ImGui::Image((ImTextureID)closedDirectoryTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.78f, 0.38f, 1.0f));
		}
		
		ImGui::SetCursorPos(positionAfterNode);

		ImGui::PopStyleColor(2);
		ImGui::PopFont();

		if (opened)
		{
			for (auto const& directoryEntry : std::filesystem::directory_iterator{ EDITOR_ASSET_DIRECTORY })
			{
				if (directoryEntry.is_directory())
					DrawDirectoryNode(directoryEntry);
			}
			ImGui::TreePop();
		}

		ImGui::PopStyleVar();
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
		if (isAncestorToAnySelectedDirectory)
			directoryNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;

		ImGuiStyle* pStyle = &ImGui::GetStyle();
		pStyle->Alpha = isSelected || isAncestorToAnySelectedDirectory ? 1.0f : 0.5f;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
		const std::string& directoryEntryName = directoryEntry.path().filename().string();

		const std::string directoryNodeName = "   " + directoryEntryName;
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

		ImGui::SetCursorPos(ImVec2(positionBeforeNode.x + 30.0f, positionBeforeNode.y + 6.0f));

		if (opened)
		{
			const std::shared_ptr<Texture> openDirectoryTexture = AssetManager::Get<Texture2D>(m_OpenDirectoryTextureHandle);
			ImGui::Image((ImTextureID)openDirectoryTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.78f, 0.38f, 1.0f));
		}
		else
		{
			const std::shared_ptr<Texture> closedDirectoryTexture = AssetManager::Get<Texture2D>(m_DirectoryTextureHandle);
			ImGui::Image((ImTextureID)closedDirectoryTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(22.0f, 22.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.58f, 0.58f, 0.58f, 1.0f));
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
		if (!ImGui::BeginChild("ContentBrowserTopChild", ImVec2(-1.0f, 30.0f), false, ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::EndChild();
			return;
		}

		if (m_SelectedHierarchyDirectories.size() > 1)
			ImGui::BeginDisabled();

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4)); // Frame padding
		
		constexpr const char* addText = "     Add";
		const ImVec2 addTextSize = ImGui::CalcTextSize(addText);
		const ImVec2 addButtonSize = ImVec2(90, 0);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); // Set border size
		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 255)); // Set border color

		const bool pressedAdd = ImGui::Button(addText, addButtonSize);
		const ImVec2 addButtonMin = ImGui::GetItemRectMin();
		const ImVec2 addButtonMax = ImGui::GetItemRectMax();

		ImGui::SameLine();
		const ImVec2 currentPosition = ImGui::GetCursorScreenPos();
		const ImVec2 plusIconPosition = ImVec2(addButtonMin.x + 16, (addButtonMin.y + addButtonMax.y) / 2.0f - 8.0f);
		ImGui::SetCursorScreenPos(plusIconPosition);
		const std::shared_ptr<Texture> plusIconTexture = AssetManager::Get<Texture2D>(m_PlusIconTextureHandle);
		ImGui::Image((ImTextureID)plusIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(16.0f, 16.0f));
		ImGui::SetCursorScreenPos(currentPosition);

		ImGui::PopStyleColor();
		ImGui::PopStyleVar(1);

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, ImVec4(0.0f,0.0f,0.0f,0.0f));
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, ImVec4(192.0f / 255, 192.0f / 255, 192.0f / 255, 1.0f));

		const float importTextWidth = ImGui::CalcTextSize("     Import").x;
		const ImVec2 buttonSize = ImVec2(90, 0);

		const bool pressed = ImGui::Button("      Import", buttonSize);
		const bool isHovered = ImGui::IsItemHovered();

		if (pressed)
			OnImportButtonPressed();

		if (isHovered)
		{
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::SetTooltip("Import assets from files to the currently selected folder");
			ImGui::PopStyleColor(2);
		}
		
		const ImVec2 importButtonMin = ImGui::GetItemRectMin();
		const ImVec2 importIconPosition = ImVec2(importButtonMin.x + 1, importButtonMin.y + 2.0f);
		ImGui::SetCursorScreenPos(importIconPosition);
		
		const std::shared_ptr<Texture> importIconTexture = AssetManager::Get<Texture2D>(m_ImportIconTextureHandle);
		ImGui::Image((ImTextureID)importIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(28.0f, 32.0f), ImVec2(0,0), ImVec2(1,1), ImVec4(192.0f / 255, 192.0f / 255, 192.0f / 255, 1.0f));

		if (m_SelectedHierarchyDirectories.size() > 1)
			ImGui::EndDisabled();

		ImGui::SameLine(0.0f, 100.0f);
		DrawHorizontalDirectoryNameList();
		
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		
		ImGui::NewLine();
		ImGui::EndChild();

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Separator] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);  // Black color
		ImGui::Separator();
	}

	void ContentBrowserPanel::RenderLeftChildWindow(float width) noexcept
	{
		constexpr ImU32 backgroundColor = IM_COL32(26.0f, 26.0f, 26.0f, 255.0f);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_ChildBg, backgroundColor);

		if (!ImGui::BeginChild("ContentBrowserLeftPanel", ImVec2(width, 0)))
		{
			ImGui::PopStyleColor();
			ImGui::EndChild();
			return;
		}

		if (ImGui::IsWindowFocused())
			m_DirectoryHierarchyFocused = true;
		else
			m_DirectoryHierarchyFocused = false;

		//RenderDirectoryHierarchySearchBar();

		RenderDirectoryHierarchy();
		
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}

	void ContentBrowserPanel::RenderRightChildWindow(float width)
	{
		if (!ImGui::BeginChild("ContentBrowserRightPanel", ImVec2(width, 0)))
		{
			ImGui::EndChild();
			return;
		}

		{
			if (!ImGui::BeginChild("ContentBrowserRightPanelTop", ImVec2(width, 45.0f)))
			{
				ImGui::EndChild();
				ImGui::EndChild();
				return;
			}

			RenderAssetHierarchyOverview();
			RenderAssetFilterButton();
			RenderAssetFilterPopup();
			RenderAssetSearchBox();
			RenderAssetThumbnailTableTopDivider();
			ImGui::EndChild();
		}

		RenderAssetThumbNails();
		RenderAssetThumbnailTableBottomDivider();
		RenderDisplayedAndSelectedEntryCountText();
		RenderPopUpOptions();

		ImGui::EndChild();
	}

	void ContentBrowserPanel::RenderDirectoryHierarchySearchBar() noexcept
	{
		UI::SearchBar("DirectoryHierarchySearchBar", "Search Paths...");
	}

	void ContentBrowserPanel::RenderAssetHierarchyOverview() noexcept
	{
		//const size_t offset = currentDirectory.string().find("Assets");
		//std::string displayLocationString = currentDirectory.string().substr(offset);
		//for (auto& character : displayLocationString)
		//{
		//	if (character == '\\')
		//		character = '/';
		//}
		//
		//std::vector<std::string> result;
		//std::istringstream iss(displayLocationString);
		//std::string token;
		//
		//std::string finalString;
		//while (std::getline(iss, token, '/')) {
		//	result.push_back(token);
		//	finalString = finalString + token;
		//	finalString = finalString + " > ";
		//}
		//
		//ImDrawList* pDrawList = ImGui::GetForegroundDrawList();
		//
		//
		//ImVec2 textPosition(m_LocationStringPosition[0], m_LocationStringPosition[1]);
		//
		//ImGuiIO& io = ImGui::GetIO();
		//auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
		//ImGui::PushFont(boldFont);
		//
		//pDrawList->AddText(textPosition, ImColor(255, 255, 255, 255), finalString.c_str());
		//ImGui::PopFont();

	}

	void ContentBrowserPanel::RenderAssetFilterButton() noexcept
	{
		const ImVec2 currentCursorPos = ImGui::GetCursorPos();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

		const bool isPressed = ImGui::Button("##FilterButton", ImVec2(56.0f, 36.0f));
		if (isPressed)
			ImGui::OpenPopup("##FilterButtonPopup");

		const bool isHovered = ImGui::IsItemHovered();
		ImGui::PopStyleVar();	
		ImGui::PopStyleColor();

		const ImVec2 buttonRectMin = ImGui::GetItemRectMin();
		ImGui::SetCursorScreenPos(ImVec2(buttonRectMin.x, buttonRectMin.y + 0.0f));

		std::shared_ptr<Texture> filterIconTexture = AssetManager::Get<Texture2D>(m_FilterIconTextureHandle);
		ImGui::Image((ImTextureID)filterIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(30.0f, 34.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(192.0f / 255, 192.0f / 255, 192.0f / 255, 1.0f));
		
		ImGui::SameLine(32.0f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

		std::shared_ptr<Texture> arrowDownIconTexture = AssetManager::Get<Texture2D>(m_DownArrowIconTextureHandle);
		ImGui::Image((ImTextureID)arrowDownIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(16.0f, 16.0f), ImVec2(0, 0), ImVec2(1, 1), ImVec4(192.0f / 255, 192.0f / 255, 192.0f / 255, 1.0f));

		if (isHovered)
		{
			ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(240.0f / 255.0f, 240.0f / 255.0f, 240.0f / 255.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::SetTooltip("Open the Add Filter Menu to add or manage filters.");
			ImGui::PopStyleColor(2);
		}

		ImGui::SameLine();
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() ,currentCursorPos.y));
	}

	void ContentBrowserPanel::RenderAssetFilterPopup() noexcept
	{
		constexpr ImU32 popupBGColor = IM_COL32(56.0f, 56.0f, 56.0f, 255.0f);
		constexpr ImU32 checkMarkBGColor = IM_COL32(36.0f, 36.0f, 36.0f, 255.0f);
		constexpr ImU32 checkMarkBorderColor = IM_COL32(67.0f, 67.0f, 67.0f, 255.0f);

		ImGui::PushStyleColor(ImGuiCol_PopupBg, popupBGColor);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, checkMarkBGColor);

		ImGui::SetNextWindowSizeConstraints
		(
			ImVec2(250.0f, 0.0f),
			ImVec2(250.0f, FLT_MAX) 
		);
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 5.0f));
		if (!ImGui::BeginPopup("##FilterButtonPopup", ImGuiWindowFlags_::ImGuiWindowFlags_NoMove))
		{
			ImGui::PopStyleColor(2);
			ImGui::PopStyleVar(1);
			return;
		}
		const float windowBeginX = ImGui::GetWindowPos().x;
		
		constexpr ImU32 filterHoverColor = IM_COL32(0.0f, 112.0f, 224.0f, 255.0f);

		const float textWidth = ImGui::CalcTextSize("Reset filters").x;
		const ImVec2 availableSize = ImGui::GetContentRegionAvail();

		const float imagePosition = ImGui::GetCursorPos().x + availableSize.x / 4.0f;
		ImGui::SetCursorPosX(imagePosition);
		const float cursorPosY = ImGui::GetCursorPos().y;
		ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 6);

		ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); 
		if (m_ActiveAssetTypeFilters.empty())
		{
			tintColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f);
			ImGui::PushStyleColor(ImGuiCol_Text, tintColor);
		}

		{
			static bool hoversResetFilterText = false;
			static ImVec2 min = {};
			static ImVec2 max = {};

			if (!m_ActiveAssetTypeFilters.empty())
			{
				if (hoversResetFilterText)
				{
					ImDrawList* pDrawList = ImGui::GetWindowDrawList();
					pDrawList->AddRectFilled(min, max, filterHoverColor);
				}
			}
			

			const std::shared_ptr<Texture2D> pUndoIcon = AssetManager::Get<Texture2D>(m_UndoArrowIconTextureHandle);
			ImGui::Image((ImTextureID)pUndoIcon->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(12, 12), ImVec2(0,0), ImVec2(1, 1), tintColor);
			ImGui::SameLine();

			ImGui::SetCursorPosX(imagePosition + 20.0f);
			ImGui::SetCursorPosY(cursorPosY);

			ImGui::Text("Reset filters");

			bool clicked = false;
			if (!m_ActiveAssetTypeFilters.empty())
			{
				min = ImVec2(windowBeginX, ImGui::GetItemRectMin().y - ImGui::GetStyle().ItemSpacing.y);
				max = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetItemRectMax().y + ImGui::GetStyle().ItemSpacing.y);
				hoversResetFilterText = ImGui::IsMouseHoveringRect(min, max);
				if (hoversResetFilterText && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					clicked = true;
					m_ActiveAssetTypeFilters.clear();
				}
			}

			if (m_ActiveAssetTypeFilters.empty() && !clicked)
				ImGui::PopStyleColor(1);
		}

		ImVec2 currentCursorPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(ImVec2(currentCursorPos.x, currentCursorPos.y + 10.0f));
		ImVec2 currentCursorScreenPos = ImGui::GetCursorScreenPos();
		UI::Utility::DrawTitledSeparator("COMMON FILTERS", ImVec2(ImGui::GetWindowPos().x, currentCursorScreenPos.y), ImVec2(250.0f, currentCursorScreenPos.y));

		ImGui::SetCursorScreenPos(ImVec2(currentCursorScreenPos.x, currentCursorScreenPos.y + 20.0f));

		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.8f);
		ImGui::PushStyleColor(ImGuiCol_Border, checkMarkBorderColor); 
		ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, checkMarkBGColor);

		{
			bool materialBoxIsChecked = m_ActiveAssetTypeFilters.contains(AssetType::Material);
			static bool materialFilterHovered = false;
			static ImVec2 min = {};
			static ImVec2 max = {};

			if (materialFilterHovered)
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				pDrawList->AddRectFilled(min, max, filterHoverColor);
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			if (ImGui::Checkbox("        Material", &materialBoxIsChecked))
				materialBoxIsChecked = !materialBoxIsChecked;

			min = ImVec2(windowBeginX, ImGui::GetItemRectMin().y - (ImGui::GetStyle().ItemSpacing.y / 2.0f));
			max = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetItemRectMax().y + (ImGui::GetStyle().ItemSpacing.y / 2.0f));
			materialFilterHovered = ImGui::IsMouseHoveringRect(min, max);
			if (materialFilterHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				materialBoxIsChecked = !materialBoxIsChecked;
				if (materialBoxIsChecked)
					m_ActiveAssetTypeFilters.insert(AssetType::Material);
				else
					m_ActiveAssetTypeFilters.erase(AssetType::Material);
			}
		}
		
		{
			bool meshBoxIsChecked = m_ActiveAssetTypeFilters.contains(AssetType::Mesh);
			static bool meshFilterHovered = false;
			static ImVec2 min = {};
			static ImVec2 max = {};

			if (meshFilterHovered)
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				pDrawList->AddRectFilled(min, max, filterHoverColor);
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			if (ImGui::Checkbox("        Mesh", &meshBoxIsChecked))
				meshBoxIsChecked = !meshBoxIsChecked;
			
			min = ImVec2(ImGui::GetWindowPos().x, ImGui::GetItemRectMin().y - (ImGui::GetStyle().ItemSpacing.y / 2.0f));
			max = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetItemRectMax().y + (ImGui::GetStyle().ItemSpacing.y / 2.0f));
			meshFilterHovered = ImGui::IsMouseHoveringRect(min, max);
			if (meshFilterHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				meshBoxIsChecked = !meshBoxIsChecked;
				if (meshBoxIsChecked)
					m_ActiveAssetTypeFilters.insert(AssetType::Mesh);
				else
					m_ActiveAssetTypeFilters.erase(AssetType::Mesh);
			}
		}
		
		{
			bool textureBoxIsChecked = m_ActiveAssetTypeFilters.contains(AssetType::Texture2D);
			static bool textureFilterHovered = false;
			static ImVec2 min = {};
			static ImVec2 max = {};

			if (textureFilterHovered)
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				pDrawList->AddRectFilled(min, max, filterHoverColor);
			}

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0f);
			if (ImGui::Checkbox("        Texture", &textureBoxIsChecked))
				textureBoxIsChecked = !textureBoxIsChecked;

			min = ImVec2(ImGui::GetWindowPos().x, ImGui::GetItemRectMin().y - (ImGui::GetStyle().ItemSpacing.y  / 2.0f));
			max = ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetItemRectMax().y + (ImGui::GetStyle().ItemSpacing.y / 2.0f));
			textureFilterHovered = ImGui::IsMouseHoveringRect(min, max);
			
			if (textureFilterHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				textureBoxIsChecked = !textureBoxIsChecked;
				if (textureBoxIsChecked)
					m_ActiveAssetTypeFilters.insert(AssetType::Texture2D);
				else
					m_ActiveAssetTypeFilters.erase(AssetType::Texture2D);
			}
		}

		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(2);

		ImGui::EndPopup();
		ImGui::PopStyleColor(2);
	}

	void ContentBrowserPanel::RenderAssetSearchBox() noexcept
	{
		m_ContentFilter = UI::SearchBar("AssetSearchBar", ConstructAssetBrowserHintString().c_str(), true, 600.0f);
	}

	void ContentBrowserPanel::RenderAssetThumbNails() noexcept
	{
		if (!ImGui::BeginChild("TableContainer", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y - 40.0f), false, ImGuiWindowFlags_HorizontalScrollbar))
		{
			ImGui::EndChild();
			return;
		}

		if (ImGui::IsWindowFocused())
			m_AssetHierarchyFocused = true;
		else
			m_AssetHierarchyFocused = false;
		
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
		{
			ImGui::EndChild();
			return;
		}

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered() && ! Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
			DeselectAllContentBrowserEntries();

		static const ImVec2 padding = ImVec2(20.0f, 10.0f);
		const float cellSize = m_ThumbnailWidth + padding.x;

		const float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);

		if (columnCount < 1)
			columnCount = 1	;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0.0f, padding.y)); // Adjust cell padding
		
		if (!ImGui::BeginTable("##MyTable", columnCount))
		{
			ImGui::PopStyleVar();
			ImGui::EndChild();
			return;
		}

		m_AssetTableYScrolledDistance = ImGui::GetScrollY();
		m_AssetTableMaxYScrolledDistance = ImGui::GetScrollMaxY();

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

				const std::string extension = dir_entry.path().filename().extension().string();

				if (extension != ".rasset")
					continue;

				const std::string entry = dir_entry.path().filename().string();
				const std::string entryPath = dir_entry.path().string();
				const AssetType assetType = AssetRegistry::GetMetaData(entryPath).AssetType;
				
				//Type filter comparison:
				if (!m_ActiveAssetTypeFilters.empty() && !m_ActiveAssetTypeFilters.contains(assetType))
					continue;

				//String filter comparison:
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
				
				if (!AssetManager::IsLoaded(entryPath))
				{
					if (AssetManager::GetAssetStatus(entryPath) == EAssetStatus::Failed)
						continue;

					{
						std::lock_guard<std::mutex> guard(m_AssetsCurrentlyBeingLoadedSetMutex);
						if (m_AssetsCurrentlyBeingLoaded.contains(entryPath))
							continue;

						m_AssetsCurrentlyBeingLoaded.insert(entryPath);
					}

					AssetManager::RequestAsyncLoadAsset(entryPath, [this, entryPath](AssetHandle assetHandle)
						{
							std::lock_guard<std::mutex> guard(m_AssetsCurrentlyBeingLoadedSetMutex);
							RLS_ASSERT(assetHandle != NULL_HANDLE, "Handle is invalid.");
							m_AssetsCurrentlyBeingLoaded.erase(entryPath);
						});

					continue;
				}

				const ImVec2 thumbnailMinPoint(ImGui::GetCursorScreenPos());
				const ImVec2 thumbnailMaxPoint(thumbnailMinPoint.x + m_ThumbnailWidth, thumbnailMinPoint.y + m_ThumbnailHeight);

				const ImVec2 vMinDropShadow(thumbnailMinPoint.x + 6.0f, thumbnailMinPoint.y + 6.0f);
				const ImVec2 vMaxDropShadow(vMinDropShadow.x + m_ThumbnailWidth, vMinDropShadow.y + m_ThumbnailHeight);
				constexpr const ImU32 black = IM_COL32(0, 0, 0, 128);
				pDrawList->AddRectFilled(vMinDropShadow, vMaxDropShadow, black, 7.0f);

				const bool isSelected = IsEntrySelected(entryPath);

				const ImVec2 windowPos = ImGui::GetWindowPos();
				const ImVec2 relativePos = ImVec2(thumbnailMinPoint.x - windowPos.x + 3.0f, thumbnailMinPoint.y - windowPos.y + ImGui::GetScrollY());

				ImGui::SetCursorPos(relativePos);
				ImGui::InvisibleButton(entry.c_str(), ImVec2(thumbnailMaxPoint.x - thumbnailMinPoint.x, thumbnailMaxPoint.y - thumbnailMinPoint.y));
				const bool isHovered = ImGui::IsItemHovered();
				if (ImGui::IsItemClicked())
				{
					const bool lCtrlPressed = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
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
						if (!isSelected)
						{
							DeselectAllContentBrowserEntries();
							SelectEntry(entryPath);
						}
					}
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left) && isHovered && isSelected)
				{
					const bool lCtrlPressed = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
					if (!lCtrlPressed)
					{
						DeselectAllContentBrowserEntries();
						SelectEntry(entryPath);
					}
				}

				DetermineEntryDragDropSource();
				
				const ImU32 unrealGrey = isSelected ? IM_COL32(30, 120, 255, 200) : isHovered ? IM_COL32(80, 80, 80, 255) : IM_COL32(60, 60, 60, 255);

				const ImVec2 rectMin = ImVec2(thumbnailMinPoint.x, thumbnailMinPoint.y + m_ThumbnailWidth);
				const ImVec2 rectMax = ImVec2(thumbnailMaxPoint.x, thumbnailMaxPoint.y);

				ImVec4 bgColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
				pDrawList->AddRectFilled(thumbnailMinPoint, thumbnailMaxPoint, IM_COL32(bgColor.x, bgColor.y, bgColor.z, bgColor.w), 5.0f);
				pDrawList->AddRectFilled(rectMin, rectMax, unrealGrey, 5.0f);

				if (isHovered || isSelected)
				{
					if (isHovered)
						ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Hand);

					const ImVec2 topLineMinPosition = ImVec2(thumbnailMinPoint.x, thumbnailMinPoint.y - 2.0f);
					const ImVec2 topLineMaxPosition = ImVec2(thumbnailMaxPoint.x, thumbnailMinPoint.y - 2.0f);
					const ImVec2 leftLineMinPosition = ImVec2(thumbnailMinPoint.x, thumbnailMinPoint.y - 2.0f + m_ThumbnailWidth);
					const ImVec2 rightLineMinPosition = ImVec2(thumbnailMaxPoint.x, thumbnailMinPoint.y - 2.0f + m_ThumbnailWidth);
					pDrawList->AddLine(topLineMinPosition, topLineMaxPosition, unrealGrey, 0.5f);
					pDrawList->AddLine(topLineMinPosition, leftLineMinPosition, unrealGrey, 0.5f);
					pDrawList->AddLine(topLineMaxPosition, rightLineMinPosition, unrealGrey, 0.5f);
				}

				const ImVec2 vMinLine(thumbnailMinPoint.x + 0.8f, thumbnailMinPoint.y + m_ThumbnailWidth);
				const ImVec2 vMaxLine(vMinLine.x + m_ThumbnailWidth - 1.6f, thumbnailMinPoint.y + m_ThumbnailWidth);

				ImU32 lineColor;
				if (assetType == AssetType::Texture2D)
					lineColor = IM_COL32(180, 0, 0, 255);
				else if (assetType == AssetType::Mesh)
					lineColor = IM_COL32(54, 214, 198, 255);
				else if (assetType == AssetType::Material)
					lineColor = IM_COL32(0, 180, 0, 255);

				pDrawList->AddLine(vMinLine, vMaxLine, lineColor, 3.0f);

				const ImVec2 assetTypeTextPosition(thumbnailMinPoint.x + 5.0f, thumbnailMaxPoint.y - 25.0f);

				ImGui::SetCursorPos(relativePos);
				const ImVec2 assetNameTextPosition = ImVec2(vMinLine.x + 7.0f, vMinLine.y + 3.0f);
				const float originalBorderSize = ImGui::GetStyle().FrameBorderSize;
				ImGui::GetStyle().FrameBorderSize = 0.0f;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

				if (assetType == AssetType::Texture2D)
				{
					const AssetHandle textureHandle = AssetManager::GetHandleByPath(entryPath);
					const std::shared_ptr<Texture2D> texture = AssetManager::Get<Texture2D>(textureHandle);
					const std::string name = texture->GetName();

					ImGui::ImageButton((ImTextureID)texture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth - 5.0f, m_ThumbnailWidth));
					ImGui::SetItemAllowOverlap();
					
					ImGui::PopStyleVar();
					ImGui::GetStyle().FrameBorderSize = originalBorderSize;

					constexpr const char* text = "Texture";
					pDrawList->AddText(assetTypeTextPosition, IM_COL32(255, 255, 255, 140), text);

					const ImVec4 fineClipRect = ImVec4(assetNameTextPosition.x, assetNameTextPosition.y, assetNameTextPosition.x + m_ThumbnailWidth - 14.0f, assetTypeTextPosition.y + 4.0f);
					const std::string displayName = std::filesystem::path(name).filename().string();
					const std::string adjustedDisplayName = UI::Utility::ShortenStringToFitClipRect(displayName, ImVec2(fineClipRect.x, fineClipRect.y), ImVec2(fineClipRect.z, fineClipRect.w));
					pDrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), assetNameTextPosition, IM_COL32(255, 255, 255, 200), adjustedDisplayName.c_str(), (const char*)0, m_ThumbnailWidth - 14.0f, &fineClipRect);
				}
				else if (assetType == AssetType::Material)
				{
					const std::shared_ptr<Texture2D> texture = AssetManager::Get<Texture2D>(m_MaterialTextureHandle);
					const std::string displayName = std::filesystem::path(entry).filename().string();

					ImGui::ImageButton((ImTextureID)texture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth - 5.0f, m_ThumbnailWidth));
					ImGui::SetItemAllowOverlap();

					ImGui::PopStyleVar();

					constexpr const char* text = "Material";
					pDrawList->AddText(assetTypeTextPosition, IM_COL32(255, 255, 255, 140), text);

					pDrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), assetNameTextPosition, IM_COL32(255, 255, 255, 200), displayName.c_str(), (const char*)0, m_ThumbnailWidth - 14.0f);
				}
				else if (assetType == AssetType::Mesh)
				{
					const std::shared_ptr<Texture2D> texture = AssetManager::Get<Texture2D>(m_MeshTextureHandle);
					const std::string displayName = std::filesystem::path(entry).filename().string();

					ImGui::ImageButton((ImTextureID)texture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth - 5.0f, m_ThumbnailWidth));
					ImGui::SetItemAllowOverlap();
					
					ImGui::PopStyleVar();

					constexpr const char* text = "Mesh";
					pDrawList->AddText(assetTypeTextPosition, IM_COL32(255, 255, 255, 140), text);

					pDrawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), assetNameTextPosition, IM_COL32(255, 255, 255, 200), displayName.c_str(), (const char*)0, m_ThumbnailWidth - 14.0f);
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
		//if (ImGui::BeginPopupContextWindow(0, 1, false /*over items*/))
		//{
		//	if (ImGui::MenuItem("Import New Asset..."))
		//	{
		//		std::filesystem::path filePath = FileDialogs::OpenFile("Image files (*.jpg,*.png)\0*.jpg;*.png\0");
		//		std::string extension = filePath.extension().string();
		//
		//		if (extension == ".jpg" || extension == ".png")
		//		{
		//			std::filesystem::copy(filePath, currentDirectory, std::filesystem::copy_options::overwrite_existing);
		//			std::filesystem::path copiedTexturePath = currentDirectory / filePath.filename();
		//			AssetManager::LoadFromFile<Texture2D>(copiedTexturePath.string());
		//		}
		//	}
		//	if (ImGui::MenuItem("New Material"))
		//	{
		//		static uint32_t createdMaterialCounter{ 0u };
		//		createdMaterialCounter = 0u;
		//		std::string name = "New Material[" + std::to_string(++createdMaterialCounter) + "]";
		//		std::filesystem::path path = currentDirectory / name;
		//		path += ".rasset";
		//		while (AssetManager::IsLoaded(path.string()))
		//		{
		//			name = "New Material[" + std::to_string(++createdMaterialCounter) + "]";
		//			path = currentDirectory / name;
		//			path.append(".rasset");
		//		}
		//
		//		m_AssetToName = AssetManager::CreateNew<Material>();
		//		Material& newMaterial = AssetManager::Get<Material>(m_AssetToName);
		//		newMaterial.SetName(name);
		//		Serializer::Serialize<Material>(m_AssetToName, path.string());
		//		
		//		AssetMetaData amd;
		//		amd.AssetType = AssetType::Material;
		//		amd.Uuid = m_AssetToName.Uuid;
		//		AssetRegistry::MapAssetToFilepath(amd, path.string());
		//	}
		//	ImGui::EndPopup();
		//}

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

	template<typename T>
	static T InverseLerp(T a, T b, T value) 
	{
		return (value - a) / (b - a);
	}

	template<typename T>
	static T Remap(T value, T inputStart, T inputEnd, T outputStart, T outputEnd) 
	{
		T t = std::clamp(InverseLerp(inputStart, inputEnd, value), T(0), T(1));
		return std::lerp(outputStart, outputEnd, t);
	}

	void ContentBrowserPanel::RenderAssetThumbnailTableTopDivider() noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		ImVec2 dividerTopLeft = ImGui::GetCursorScreenPos();
		dividerTopLeft.y += 3.0f;
		const ImVec2 dividerBottomRight = ImVec2(dividerTopLeft.x + ImGui::GetContentRegionAvail().x, dividerTopLeft.y + 10.0f);

		const ImVec4 colorTopLeft = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		const ImVec4 colorTopRight = colorTopLeft;

		const float alpha = Remap<float>(m_AssetTableYScrolledDistance, 0.0f, 5.0f, 0.0f, 255.0f);
		const ImU32 colorBottomLeft = IM_COL32(20, 20, 20, alpha);
		const ImU32 colorBottomRight = IM_COL32(20, 20, 20, alpha);

		pDrawList->AddRectFilledMultiColor
		(
			dividerTopLeft,
			dividerBottomRight,
			colorBottomRight,
			colorBottomLeft,
			ImGui::ColorConvertFloat4ToU32(colorTopLeft),
			ImGui::ColorConvertFloat4ToU32(colorTopRight)
		);
	}

	void ContentBrowserPanel::RenderAssetThumbnailTableBottomDivider() noexcept
	{
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		ImVec2 dividerTopLeft = ImGui::GetCursorScreenPos();
		dividerTopLeft.y -= 14.0f;
		const ImVec2 dividerBottomRight = ImVec2(dividerTopLeft.x + ImGui::GetContentRegionAvail().x, dividerTopLeft.y + 10.0f);

		const ImVec4 colorTopLeft = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		const ImVec4 colorTopRight = colorTopLeft;
		constexpr ImU32 colorBottomLeft = IM_COL32(20, 20, 20, 255);
		constexpr ImU32 colorBottomRight = IM_COL32(20, 20, 20, 255);

		pDrawList->AddRectFilledMultiColor
		(
			dividerTopLeft, 
			dividerBottomRight, 
			ImGui::ColorConvertFloat4ToU32(colorTopLeft), 
			ImGui::ColorConvertFloat4ToU32(colorTopRight), 
			colorBottomRight, 
			colorBottomLeft
		);
	}

	void ContentBrowserPanel::RenderDisplayedAndSelectedEntryCountText() noexcept
	{
		const std::string entryDisplayText = std::format
		("{} {} {}",
			m_DisplayedEntries,
			(m_DisplayedEntries > 1 || m_DisplayedEntries == 0) ? "items" : "item",
			m_SelectedEntries.size() > 0 ? " (" + std::to_string(m_SelectedEntries.size()) + " selected)" : ""
		);
		
		ImGui::Text(entryDisplayText.c_str());
	}

	void ContentBrowserPanel::RenderChildWindowsDraggableSplitter(float splitterThickness) noexcept
	{
		ImGui::SameLine();

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() - splitterThickness);

		constexpr const ImVec4 splitterColor = ImVec4(22.0f / 255, 22.0f / 255, 22.0f / 255, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, splitterColor);
		
		ImGui::Button("##Splitter", ImVec2(splitterThickness, -1));
		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);
		if (ImGui::IsItemActive())
			m_DragAmount += ImGui::GetIO().MouseDelta.x;

		ImGui::PopStyleColor();

		ImGui::SameLine();
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
			std::string materialName = AssetManager::Get<Material>(handle)->GetName();
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
				const std::string materialNametoReplace = AssetManager::Get<Material>(handle)->GetName();
				const std::string materialFilename = materialNametoReplace + ASSET_EXTENSION;
				AssetManager::Get<Material>(handle)->SetName(newName);

				std::filesystem::path fullPathToSave = currentDirectory;
				fullPathToSave.append(std::string(newName) + ASSET_EXTENSION);
				std::filesystem::path fullPathToDelete = currentDirectory;
				fullPathToDelete.append(materialFilename);

				Serializer::Serialize<Material>(m_AssetToName, fullPathToSave.string(), true);
				std::filesystem::remove(fullPathToDelete.string());
				memset(newName, 0, 64);

				AssetManager::OnFileMoved(m_AssetToName, fullPathToSave.string());
				AssetRegistry::RemoveUUIDToFilepathMap(m_AssetToName.Uuid, fullPathToDelete.string());

				RLS_ASSERT(false, "TODO!!");
				//AssetRegistry::MapAssetToFilepath({ m_AssetToName.Uuid, AssetType::Material }, fullPathToSave.string());
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

		const std::string directoryName = directoryPath.filename().string();
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
				if (!isSelected && lCtrlDown)
					SelectEntry(directoryPath.string());
				else
				{
					DeselectAllContentBrowserEntries();
					SelectEntry(directoryPath.string());
				}
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

		ImGui::ImageButton((void*)AssetManager::Get<Texture2D>(m_DirectoryTextureHandle)->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(m_ThumbnailWidth, m_ThumbnailWidth), ImVec2(0, 0), ImVec2(1, 1), -1, ImVec4(0, 0, 0, 0), ImVec4(0.58f, 0.58f, 0.58f, 1.0f));

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

	void ContentBrowserPanel::SelectAllEntriesInDirectories(const std::vector<std::string>& directories) noexcept
	{
		for (auto& directoryPath : directories)
		{
			if (!File::ExistsDir(directoryPath))
				continue;

			for (auto const& dir_entry : std::filesystem::directory_iterator{ directoryPath })
			{
				const std::filesystem::path entryPath = dir_entry.path();

				if (IsEntrySelected(entryPath.string()))
					continue;

				if (FilepathUtils::IsDirectory(entryPath))
					m_SelectedEntries.push_back(entryPath.string());
				else
				{
					const bool isAssetFile = FilepathUtils::ExtractExtension(entryPath) == ASSET_EXTENSION;
					if (!isAssetFile)
						continue;

					const AssetType assetType = AssetRegistry::GetMetaData(entryPath).AssetType;
					const bool isFiltering = m_ActiveAssetTypeFilters.size() > 0;

					if (isFiltering && !m_ActiveAssetTypeFilters.contains(assetType))
						continue;

					m_SelectedEntries.push_back(entryPath.string());
				}
			}
		}
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

		const std::string firstDirectoryName = std::filesystem::path(m_SelectedHierarchyDirectories[0]).filename().string();
		std::string constructedHint = "Search ";
		constructedHint += firstDirectoryName.empty() ? "Content" : firstDirectoryName;

		for (uint32_t i{ 1u }; i < m_SelectedHierarchyDirectories.size(); ++i)
		{
			constructedHint += ", ";
			const std::string directoryName = std::filesystem::path(m_SelectedHierarchyDirectories[i]).filename().string();
			constructedHint += directoryName.empty() ? "Content" : directoryName;
		}
		
		return std::move(constructedHint);
	}

	void ContentBrowserPanel::OnImportButtonPressed() noexcept
	{
		constexpr const char* filter = "All Files (*.png;*.jpg;*.tga;*.fbx;*.obj;*.gltf;*.glb)\0*.png;*.jpg;*.tga;*.fbx;*.obj;*.gltf;*.glb\0";

		const std::vector<std::string> filepaths = FileDialogs::OpenFile(filter);
		if (filepaths.empty())
			return;

		for (auto& filePath : filepaths)
			Importer::RequestAsyncLoadFromFile(filePath, currentDirectory);
	}

	void ContentBrowserPanel::DetermineEntryDragDropSource() noexcept
	{
		const uint32_t selectedEntryCount = m_SelectedEntries.size();
		if (selectedEntryCount == 0u)
			return;

		if (selectedEntryCount == 1)
		{
			if (FilepathUtils::IsDirectory(m_SelectedEntries[0]))
				return;

			const AssetType assetType = AssetRegistry::GetMetaData(m_SelectedEntries[0]).AssetType;
			AssetHandle handle = AssetManager::GetHandleByPath(m_SelectedEntries[0]);
			switch (assetType)
			{
			case AssetType::Material:
			{
				std::shared_ptr<Texture2D> pTexture = AssetManager::Get<Texture2D>(m_MaterialTextureHandle);
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("MATERIAL_DRAG_DROP", &handle, sizeof(AssetHandle));
					ImGui::Image((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(64,64));
					ImGui::EndDragDropSource();
				}
				break;
			}
			case AssetType::Mesh:
			{
				std::shared_ptr<Texture2D> pTexture = AssetManager::Get<Texture2D>(m_MeshTextureHandle);
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("MESH_DRAG_DROP", &handle, sizeof(AssetHandle));
					ImGui::Image((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(64, 64));
					ImGui::EndDragDropSource();
				}
				break;
			}
			case AssetType::Texture2D:
			{
				std::shared_ptr<Texture2D> pTexture = AssetManager::Get<Texture2D>(handle);
				if (ImGui::BeginDragDropSource())
				{
					ImGui::SetDragDropPayload("TEXTURE2D_DRAG_DROP", &handle, sizeof(AssetHandle));
					ImGui::Image((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(64, 64));
					ImGui::EndDragDropSource();
				}
				break;
			}
			}
		}
		else
		{
			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("MULTIPLE_ENTRIES_DRAG_DROP", nullptr, 0);

				ImGui::Text("%d assets", selectedEntryCount);
				ImGui::EndDragDropSource();
			}
		}
	}

	std::vector<std::filesystem::path> ContentBrowserPanel::ConstructSubPaths() const noexcept
	{
		std::vector<std::filesystem::path> pathsToReturn;
		
		std::filesystem::path path = currentDirectory;
		while (FilepathUtils::IsDirectory(path) && path.has_parent_path())
		{
			pathsToReturn.push_back(path);
			const std::string filename = FilepathUtils::ExtractFilenameWithoutExtension(path);
			path = path.parent_path();

			if (filename == "Assets")
				break;
		}

		std::reverse(pathsToReturn.begin(), pathsToReturn.end());

		return pathsToReturn;
	}

	void ContentBrowserPanel::DrawHorizontalDirectoryNameList()
	{
		const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

		// Convert the screen position to window-relative position
		ImVec2 cursorWindowPos = ImVec2(cursorScreenPos.x - ImGui::GetWindowPos().x, cursorScreenPos.y - ImGui::GetWindowPos().y);
		cursorWindowPos.y -= 2.0f;

		// Set the cursor position to the calculated window-relative position
		ImGui::SetCursorPos(cursorWindowPos);

		const std::vector<std::filesystem::path> projectSubpaths = ConstructSubPaths();

		float maxButtonHeight = 0.0f;
		for (const auto& path : projectSubpaths)
		{
			const std::string filename = FilepathUtils::ExtractFilenameWithoutExtension(path);
			ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());
			if (textSize.y > maxButtonHeight)
			{
				maxButtonHeight = textSize.y;
			}
		}

		constexpr float arrowImageHeight = 20.0f;
		const float arrowImageY = cursorWindowPos.y + (maxButtonHeight - arrowImageHeight) / 2.0f + 8.0f;

		for (auto& path : projectSubpaths)
		{
			const std::string filename = FilepathUtils::ExtractFilenameWithoutExtension(path);

			const float buttonWidth = ImGui::CalcTextSize(filename.c_str()).x + 10;
			if (ImGui::Button(filename.c_str(), ImVec2(buttonWidth, maxButtonHeight + 10)))
			{
				DeselectAllHierarchyDirectories();
				SelectHiearchyDirectory(path.string());
				currentDirectory = path;
			}

			ImGui::SameLine();

			const std::shared_ptr<Texture2D> pRightArrowTexture = AssetManager::Get<Texture2D>(m_RightArrowIconTextureHandle);
			ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), arrowImageY));
			
			ImGui::Image((ImTextureID)pRightArrowTexture->GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(20.0f, 20.0f), ImVec2(0,0), ImVec2(1,1), ImVec4(192.0f / 255, 192.0f / 255, 192.0f / 255, 1.0f));
			ImGui::SameLine();
		}
	}
}