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
		void RenderDirectoryHierarchySearchBox() noexcept;
		void RenderAssetHierarchyOverview() noexcept;
		void RenderAssetSearchBox() noexcept;
		void RenderAssetThumbNails() noexcept;
		void RenderPopUpOptions() noexcept;
		void RenderThumbnailText(const std::string& text, bool thumbNailHovered, const AssetHandle& handle = NULL_HANDLE) noexcept;
	private:
		float m_ThumbnailWidth;
		TextureHandle m_DirectoryTextureHandle;
		TextureHandle m_SceneTextureHandle;
		TextureHandle m_MaterialTextureHandle;
		std::string m_SelectedDirectory;
		float m_LocationStringPosition[2];
		uint32_t m_NrOfCreatedMaterials{ 0u };

		AssetHandle m_AssetToName;

		std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> m_OnAssetSelectedCallback;
	};
}