#pragma once
#include <Relentless.h>

namespace Relentless
{
	class OutlinerEntityTableData : public TableData 
	{
	public:
		OutlinerEntityTableData(entity id, Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept;
		virtual ~OutlinerEntityTableData() noexcept override = default;

		[[nodiscard]] const char* GetColumnString(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] const char* GetColumnTooltip(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] AssetHandle GetColumnIcon(uint32_t columnIndex) const noexcept override;

		[[nodiscard]] entity GetEntityID() const noexcept;
	private:
		Scene* m_pOwningScene = nullptr;
		entity m_EntityID = NULL_ENTITY;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;
	};

	class OutlinerSceneTableData : public TableData
	{
	public:
		OutlinerSceneTableData(Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept;
		virtual ~OutlinerSceneTableData() noexcept override = default;
		[[nodiscard]] const char* GetColumnString(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] const char* GetColumnTooltip(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] AssetHandle GetColumnIcon(uint32_t columnIndex) const noexcept override;

		[[nodiscard]] Scene* GetScene() noexcept;
	private:
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;
	};
}