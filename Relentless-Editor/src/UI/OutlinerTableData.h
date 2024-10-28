#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ETableDataType : uint8_t { None = 0, Scene, Entity, Folder };

	class OutlinerTableData : public TableData
	{
	public:
		OutlinerTableData(Table* pTable, ETableDataType type) noexcept;
		virtual ~OutlinerTableData() noexcept override = default;
		[[nodiscard]] virtual bool IsVisible() const noexcept = 0;
		virtual void SetAndPropagateVisibleState(bool visibleState) noexcept = 0;
		[[nodiscard]] ETableDataType GetType() const noexcept;
	private:
		ETableDataType m_Type = ETableDataType::None;
	};

	class OutlinerEntityTableData : public OutlinerTableData
	{
	public:
		OutlinerEntityTableData(Table* pTable, entity id, Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept;
		virtual ~OutlinerEntityTableData() noexcept override = default;

		[[nodiscard]] const char* GetColumnString(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] const char* GetColumnTooltip(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] AssetHandle GetColumnIcon(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] virtual bool IsVisible() const noexcept override;
		virtual void SetAndPropagateVisibleState(bool visibleState) noexcept override;
		[[nodiscard]] entity GetEntityID() const noexcept;
	private:
		Scene* m_pOwningScene = nullptr;
		entity m_EntityID = NULL_ENTITY;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;
	};

	class OutlinerSceneTableData : public OutlinerTableData
	{
	public:
		OutlinerSceneTableData(Table* pTable, Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept;
		virtual ~OutlinerSceneTableData() noexcept override = default;
		[[nodiscard]] const char* GetColumnString(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] const char* GetColumnTooltip(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] AssetHandle GetColumnIcon(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] virtual bool IsVisible() const noexcept override;
		virtual void SetAndPropagateVisibleState(bool visibleState) noexcept override;

		[[nodiscard]] Scene* GetScene() noexcept;
	private:
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;
	};

	class OutlinerFolderTableData : public OutlinerTableData
	{
	public:
		OutlinerFolderTableData(Table* pTable, Scene* pScene, const char* name, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept;
		virtual ~OutlinerFolderTableData() noexcept override = default;
		[[nodiscard]] const char* GetColumnString(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] const char* GetColumnTooltip(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] AssetHandle GetColumnIcon(uint32_t columnIndex) const noexcept override;
		[[nodiscard]] virtual bool IsVisible() const noexcept override;
		virtual void SetAndPropagateVisibleState(bool visibleState) noexcept override;

	private:
		std::string m_Name = nullptr;
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;
	};
}