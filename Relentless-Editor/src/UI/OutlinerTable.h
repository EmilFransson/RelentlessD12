#pragma once
#include <Relentless.h>

namespace Relentless
{
	class OutlinerTable : public Table
	{
	public:
		OutlinerTable() noexcept;
		virtual ~OutlinerTable() noexcept override = default;

		void AddEntry(const std::shared_ptr<TableData>& pTableData) noexcept override;
		void OnSceneChanged(Scene* pScene) noexcept;
	private:
		virtual [[nodiscard]] const char* GetID() const noexcept override;
	private:
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;
	};
}