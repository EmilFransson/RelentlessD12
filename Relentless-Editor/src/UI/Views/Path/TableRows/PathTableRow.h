#pragma once
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	struct PathListItem : public RefCounted<PathListItem>
	{
		String VirtualPath;
		String DisplayName;
		EAssetSourceType SourceType = EAssetSourceType::Project;
		bool IsRoot					= false;
	};

	class Button;
	class Label;
	template<typename T> class TreeView;

	struct PathTableRowCreateInfo
	{
		String DisplayName							= "";
		String Tooltip								= "";
		String HighlightText						= "";
		bool IsExpanded								= false;
		bool HasChildren							= false;
		TreeView<Ref<PathListItem>>* OwningTreeView = nullptr;
	};

	class PathTableRow : public ITableRow
	{
	public:
		PathTableRow(const PathTableRowCreateInfo& aCreateInfo) noexcept;

		NO_DISCARD const Color& GetBackgroundColor() const noexcept override;
		NO_DISCARD uint32 GetNumColumns() noexcept override;
	protected:
		virtual void OnRenderColumn(uint32 aColumn) noexcept override;
	private:
		void OnChevronButtonClicked() noexcept;
	private:
		TreeView<Ref<PathListItem>>* m_pOwningTreeView = nullptr;
		Button* m_pChevronButton = nullptr;
	};
}