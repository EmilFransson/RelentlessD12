#pragma once
#include <Relentless.h>
#include "OutlinerTableData.h"

namespace Relentless
{
	class OutlinerTable : public Table
	{
	public:
		OutlinerTable() noexcept;
		virtual ~OutlinerTable() noexcept override = default;

		void OnSceneChanged(Scene* pScene) noexcept;
		void AddEntityEntry(entity e, const std::unique_ptr<TableDataSlice>& pSlice = nullptr) noexcept;
		void AddFolderEntry(const char* name) noexcept;
		void SelectAllExpandedEntityRows() noexcept;
		void SelectEntity(entity e) noexcept;
		void DeselectEntity(entity e) noexcept;
		void DeselectAllEntities() noexcept;


		[[nodiscard]] bool IsEntitySelected(entity e) const noexcept;
		[[nodiscard]] uint32_t GetNrOfEntityEntries() const noexcept;
		[[nodiscard]] uint32_t GetNrOfSelectedEntities() const noexcept;
		[[nodiscard]] const std::vector<entity>& GetSelectedEntities() const noexcept;

		[[nodiscard]] Scene* GetScene() noexcept;
	private:
		virtual [[nodiscard]] const char* GetID() const noexcept override;
	private:
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;

		std::unordered_map<entity, std::shared_ptr<OutlinerEntityTableData>> m_EntityIDToTableDataEntry;
	};
}