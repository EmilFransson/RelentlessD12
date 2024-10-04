#pragma once
#include <Relentless.h>

namespace Relentless
{
	class OutlinerTable : public Table
	{
	public:
		OutlinerTable() noexcept;
		virtual ~OutlinerTable() noexcept override = default;

		void OnSceneChanged(Scene* pScene) noexcept;
		void AddEntityEntry(entity e) noexcept;
		void SelectAllExpandedEntityRows() noexcept;

		[[nodiscard]] uint32_t GetNrOfEntityEntries() const noexcept;
		[[nodiscard]] uint32_t GetNrOfSelectedEntities() const noexcept;

		[[nodiscard]] Scene* GetScene() noexcept;
	private:
		virtual [[nodiscard]] const char* GetID() const noexcept override;
	private:
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;

		uint32_t m_NrOfEntityEntries = 0u;
	};
}