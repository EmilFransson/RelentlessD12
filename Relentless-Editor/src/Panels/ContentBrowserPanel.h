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
		void SetOnAssetSelectedCallback(std::function<void(const ResourceID& resourceID, const InspectedAssetType inspectedAssetType)> callback) noexcept;
	private:
		void RenderDirectoryHierarchy() noexcept;
		void DrawDirectoryNode(const std::filesystem::directory_entry& directoryName) noexcept;
		void RenderMenuBar() noexcept;
		void RenderDirectoryHierarchySearchBox() noexcept;
		void RenderAssetHierarchyOverview() noexcept;
		void RenderAssetSearchBox() noexcept;
		void RenderAssetThumbNails() noexcept;
		void RenderPopUpOptions() noexcept;
		void RenderThumbnailText(const std::string& text, bool thumbNailHovered) noexcept;
	private:
		float m_ThumbnailWidth;
		std::shared_ptr<Texture2D> m_pDirectoryTexture;
		std::shared_ptr<Texture2D> m_pSceneTexture;
		std::shared_ptr<Texture2D> m_pMaterialTexture;
		std::string m_SelectedDirectory;
		float m_LocationStringPosition[2];

		std::unordered_map<std::string, std::vector<MaterialHandle>> m_CreatedMaterials;

		std::function<void(const ResourceID& resourceID, const InspectedAssetType inspectedAssetType)> m_OnAssetSelectedCallback;
	};
}