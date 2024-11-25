#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class ETreeItemType : uint8_t { None = 0, Scene, Entity, Filter };

	class OutlinerTreeItem : public TreeItem
	{
	public:
		OutlinerTreeItem(ETreeItemType type, TreeItem* pParent = nullptr) noexcept;
		virtual ~OutlinerTreeItem() noexcept override = default;

		void SetVisibility(bool visibilityState) noexcept;
		void SetVisibilityIcon(const AssetHandle& iconHandle) noexcept;

		[[nodiscard]] ETreeItemType GetType() const noexcept;
		[[nodiscard]] bool IsVisible() const noexcept;
	private:
		ETreeItemType m_Type = ETreeItemType::None;
		bool m_IsVisible = true;
	};

	class OutlinerEntityTreeItem : public OutlinerTreeItem
	{
	public:
		OutlinerEntityTreeItem(entity id) noexcept;
		virtual ~OutlinerEntityTreeItem() noexcept override = default;

		//virtual void SetAndPropagateVisibleState(bool visibleState) noexcept override;
		[[nodiscard]] entity GetEntityID() const noexcept;
	private:
		entity m_EntityID = NULL_ENTITY;
	};

	class OutlinerSceneTreeItem : public OutlinerTreeItem
	{
	public:
		OutlinerSceneTreeItem(Scene* pScene) noexcept;
		virtual ~OutlinerSceneTreeItem() noexcept override = default;
		//virtual void SetAndPropagateVisibleState(bool visibleState) noexcept override;

		[[nodiscard]] Scene* GetScene() noexcept;
	private:
		Scene* m_pScene = nullptr;
	};

	class OutlinerFolderTreeItem : public OutlinerTreeItem
	{
	public:
		OutlinerFolderTreeItem(const char* name) noexcept;
		virtual ~OutlinerFolderTreeItem() noexcept override = default;
		//virtual void SetAndPropagateVisibleState(bool visibleState) noexcept override;
	private:
		std::string m_Name = nullptr;
	};
}