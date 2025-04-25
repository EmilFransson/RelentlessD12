#pragma once
#pragma once
#include <Relentless.h>
#include "InspectorPanel.h"
namespace Relentless
{
	class ContentBrowserPanel
	{
	public:
		explicit ContentBrowserPanel() noexcept;
		~ContentBrowserPanel() noexcept = default;
		void OnImGuiRender(const bool show) noexcept;
		void OnEvent(IEvent& event) noexcept;
		void SetOnAssetSelectedCallback(std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> callback) noexcept;
		[[nodiscard]] const std::vector<std::string>& GetSelectedEntries() const noexcept;
	private:
		void RenderDirectoryHierarchy() noexcept;
		void DrawDirectoryNode(const std::filesystem::directory_entry& directoryName) noexcept;
		void RenderMenuBar() noexcept;
		void RenderLeftChildWindow(float width) noexcept;
		void RenderRightChildWindow(float width);
		void RenderDirectoryHierarchySearchBar() noexcept;
		void RenderAssetHierarchyOverview() noexcept;
		void RenderAssetFilterButton() noexcept;
		void RenderAssetFilterPopup() noexcept;
		void RenderAssetSearchBox() noexcept;
		void RenderAssetThumbNails() noexcept;
		void RenderPopUpOptions() noexcept;
		void RenderThumbnailText(const std::string& text, bool thumbNailHovered) noexcept;
		void RenderAssetThumbnailTableTopDivider() noexcept;
		void RenderAssetThumbnailTableBottomDivider() noexcept;
		void RenderDisplayedAndSelectedEntryCountText() noexcept;
		void RenderChildWindowsDraggableSplitter(float splitterThickness) noexcept;
		void EditThumbnailText(const AssetHandle& handle) noexcept;
		void DrawDirectoryThumbnail(const std::filesystem::path directoryPath) noexcept;
		
		[[nodiscard]] bool IsEntrySelected(const std::string& entryPath) const noexcept;
		void SelectEntry(const std::string& entryPath) noexcept;
		void DeselectEntry(const std::string& entryPath) noexcept;
		void SelectAllEntriesInDirectories(const std::vector<std::string>& directories) noexcept;

		[[nodiscard]] bool IsDirectorySelectedInHierarchy(const std::string& directoryPath) const;
		void SelectHiearchyDirectory(const std::string& directoryPath) noexcept;
		void DeselectHiearchyDirectory(const std::string& directoryPath) noexcept;
		void DeselectAllHierarchyDirectories() noexcept;
		void DeselectAllContentBrowserEntries() noexcept;
		[[nodiscard]] uint32 GetSelectedHierarchyDirectoriesCount() const;
		[[nodiscard]] bool IsAncestorDirectoryToAnySelectedDirectory(const std::filesystem::path& directoryPath) const;
		[[nodiscard]] std::string ConstructAssetBrowserHintString() const noexcept;

		void OnImportButtonPressed() noexcept;
		void DetermineEntryDragDropSource() noexcept;

		[[nodiscard]] std::vector<std::filesystem::path> ConstructSubPaths() const noexcept;
		void DrawHorizontalDirectoryNameList();
	private:
		float m_ThumbnailWidth = 150.0f;
		float m_ThumbnailHeight = 220.0f;

		AssetHandle m_DirectoryTextureHandle = NULL_HANDLE;
		AssetHandle m_OpenDirectoryTextureHandle = NULL_HANDLE;
		AssetHandle m_SceneTextureHandle = NULL_HANDLE;
		AssetHandle m_MaterialTextureHandle = NULL_HANDLE;
		AssetHandle m_MeshTextureHandle = NULL_HANDLE;
		AssetHandle m_ImportIconTextureHandle = NULL_HANDLE;
		AssetHandle m_FilterIconTextureHandle = NULL_HANDLE;
		AssetHandle m_DownArrowIconTextureHandle = NULL_HANDLE;
		AssetHandle m_UndoArrowIconTextureHandle = NULL_HANDLE;
		AssetHandle m_PlusIconTextureHandle = NULL_HANDLE;
		AssetHandle m_RightArrowIconTextureHandle = NULL_HANDLE;
		UUID m_SelectedAsset = NULL_UUID;

		std::string m_ContentFilter = "";

		std::string m_SelectedDirectory = "Assets";
		float m_LocationStringPosition[2];
		bool m_FirstTimeEditingThumbnail{ true };
		std::vector<std::string> m_SelectedEntries;
		std::vector<std::string> m_SelectedHierarchyDirectories;

		AssetHandle m_AssetToName = NULL_HANDLE;

		bool m_DirectoryHierarchyFocused = false;
		bool m_AssetHierarchyFocused = false;
		float m_DragAmount = 0.0f;
		uint32_t m_DisplayedEntries = 0u;

		std::unordered_set<AssetType> m_ActiveAssetTypeFilters;

		float m_AssetTableYScrolledDistance = 0.0f;
		float m_AssetTableMaxYScrolledDistance = 0.0f;

		std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> m_OnAssetSelectedCallback;

		std::mutex m_AssetsCurrentlyBeingLoadedSetMutex;
		std::set<std::filesystem::path> m_AssetsCurrentlyBeingLoaded;
	};
}