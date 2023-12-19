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
		void SetActiveScene(std::shared_ptr<Scene> pScene) noexcept;
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
		void EditThumbnailText(const AssetHandle& handle) noexcept;

		template<typename AssetType>
		void RenderAssetThumbnail(const AssetHandle& imageButtonTextureHandle, const AssetHandle& tooltipImageTextureHandle, const AssetHandle& payloadAssetHandle, const std::string& name, float thumbnailWidth) noexcept
		{
			ImGui::PushID(ConvertUUIDToString(payloadAssetHandle.Uuid).c_str());
			Texture2D& imageButtonTexture = AssetManager::Get<Texture2D>(imageButtonTextureHandle);
			ImGui::ImageButton((ImTextureID)imageButtonTexture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(thumbnailWidth, thumbnailWidth));

			Texture2D& tooltipImageTexture = AssetManager::Get<Texture2D>(tooltipImageTextureHandle);

			constexpr std::string_view payloadID = std::is_same_v<AssetType, Texture2D> ? "CONTENT_BROWSER_ITEM_TEXTURE" 
				: std::is_same_v<AssetType, Material> ? "CONTENT_BROWSER_ITEM_MATERIAL" : "CONTENT_BROWSER_ITEM_SCENE";

			if (ImGui::BeginDragDropSource())
			{
				ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
				ImGui::BeginTooltip();
				ImGui::Image((ImTextureID)tooltipImageTexture.GetSRVDescriptorHandle().GPUHandle.ptr, ImVec2(thumbnailWidth, thumbnailWidth));
				ImGui::EndTooltip();
				ImGui::PopStyleVar(1);
				ImGui::SetDragDropPayload(payloadID.data(), (void*)&payloadAssetHandle, sizeof(AssetHandle), ImGuiCond_::ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}
			else if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				if constexpr (std::is_same_v<AssetType, Material>)
				{
					m_OnAssetSelectedCallback(payloadAssetHandle, InspectedAssetType::MATERIAL);
				}
			}

			ImGui::SameLine();
			ImGui::NewLine();

			if (payloadAssetHandle == m_AssetToName && !std::is_same_v<AssetType, Texture2D>)
			{
				EditThumbnailText(payloadAssetHandle);
			}
			else
			{
				RenderThumbnailText(name, ImGui::IsItemHovered());
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					m_AssetToName = payloadAssetHandle;
				}
			}

			ImGui::PopID();
		}

	private:
		float m_ThumbnailWidth;
		AssetHandle m_DirectoryTextureHandle;
		AssetHandle m_SceneTextureHandle;
		AssetHandle m_MaterialTextureHandle;
		std::string m_SelectedDirectory;
		float m_LocationStringPosition[2];
		bool m_FirstTimeEditingThumbnail{ true };

		std::shared_ptr<Scene> m_pScene;

		AssetHandle m_AssetToName;

		std::function<void(const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)> m_OnAssetSelectedCallback;
		AssetHandle aex;
	};
}