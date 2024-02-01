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
		void SetOnAssetSelectedCallback(std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> callback) noexcept;
	private:
		void RenderDirectoryHierarchy() noexcept;
		void DrawDirectoryNode(const std::filesystem::directory_entry& directoryName) noexcept;
		void RenderMenuBar() noexcept;
		void RenderDirectoryHierarchySearchBar() noexcept;
		void RenderAssetHierarchyOverview() noexcept;
		void RenderAssetSearchBox() noexcept;
		void RenderAssetThumbNails() noexcept;
		void RenderPopUpOptions() noexcept;
		void RenderThumbnailText(const std::string& text, bool thumbNailHovered) noexcept;
		void EditThumbnailText(const AssetHandle& handle) noexcept;
		void DrawDirectoryThumbnail(const std::filesystem::path directoryPath) noexcept;
		
		[[nodiscard]] bool IsEntrySelected(const std::string& entryPath) const noexcept;
		void SelectEntry(const std::string& entryPath) noexcept;
		void DeselectEntry(const std::string& entryPath) noexcept;

		[[nodiscard]] bool IsDirectorySelectedInHierarchy(const std::string& directoryPath) const;
		void SelectHiearchyDirectory(const std::string& directoryPath) noexcept;
		void DeselectHiearchyDirectory(const std::string& directoryPath) noexcept;
		void DeselectAllHierarchyDirectories() noexcept;
		void DeselectAllContentBrowserEntries() noexcept;
		[[nodiscard]] uint32_t GetSelectedHierarchyDirectoriesCount() const;
		[[nodiscard]] bool IsAncestorDirectoryToAnySelectedDirectory(const std::filesystem::path& directoryPath) const;
		[[nodiscard]] std::string ConstructAssetBrowserHintString() const noexcept;
	private:
		float m_ThumbnailWidth = 150.0f;
		float m_ThumbnailHeight = 220.0f;

		AssetHandle m_DirectoryTextureHandle = NULL_HANDLE;
		AssetHandle m_OpenDirectoryTextureHandle = NULL_HANDLE;
		AssetHandle m_SceneTextureHandle = NULL_HANDLE;
		AssetHandle m_MaterialTextureHandle = NULL_HANDLE;
		AssetHandle m_MeshTextureHandle = NULL_HANDLE;
		UUID m_SelectedAsset = NULL_UUID;

		std::string m_ContentFilter = "";

		std::string m_SelectedDirectory = "Assets";
		float m_LocationStringPosition[2];
		bool m_FirstTimeEditingThumbnail{ true };
		std::vector<std::string> m_SelectedEntries;
		std::vector<std::string> m_SelectedHierarchyDirectories;

		AssetHandle m_AssetToName = NULL_HANDLE;

		bool m_DirectoryHierarchyFocused = false;
		float m_DragAmount = 0.0f;
		uint32_t m_DisplayedEntries = 0u;

		std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> m_OnAssetSelectedCallback;
	};
}