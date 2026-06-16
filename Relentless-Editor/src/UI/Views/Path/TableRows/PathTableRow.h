#pragma once
#include <Relentless.h>

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
	class EditableTextBox;
	class Label;
	template<typename T> class TreeView;
	class WidgetSwitcher;

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
		NO_DISCARD Ref<PathListItem> GetItem() const noexcept;
		NO_DISCARD uint32 GetNumColumns() noexcept override;

		template<typename InstanceType>
		PathTableRow* OnRenameTextChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(const Ref<PathListItem>&, const char*)) noexcept;

		template<typename InstanceType>
		PathTableRow* OnRenameTextCommitted(InstanceType* aInstance, void(InstanceType::*aMethod)(const Ref<PathListItem>&, const char*, ETextCommitType)) noexcept;

		void SetRenameFieldError(bool aIsError) noexcept;
		void ShowEditableTextBox() noexcept;
		void ShowLabel() noexcept;
	protected:
		void HandleDragDrop() noexcept override;

		void OnChevronButtonClicked() noexcept;
		virtual void OnRenderColumn(uint32 aColumn) noexcept override;

		virtual bool SupportsDrag() const noexcept override { return true; }
	private:
		void OnRenameTextChangedInternal(const char* aText) noexcept;
		void OnRenameTextCommittedInternal(const char* aText, ETextCommitType aTextCommitType) noexcept;
	private:
		Callback<void(const Ref<PathListItem>&, const char*)> OnRenameTextChangedCallback;
		Callback<void(const Ref<PathListItem>&, const char*, ETextCommitType)> OnRenameTextCommittedCallback;
		
		TreeView<Ref<PathListItem>>* m_pOwningTreeView = nullptr;
		Button* m_pChevronButton = nullptr;
		EditableTextBox* m_pEditableTextBox = nullptr;
		WidgetSwitcher* m_pSwitcher = nullptr;
		Label* m_pExclamationLabel = nullptr;
	};

	template<typename InstanceType>
	PathTableRow* PathTableRow::OnRenameTextChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(const Ref<PathListItem>&, const char*)) noexcept
	{
		OnRenameTextChangedCallback = [aInstance, aMethod](const Ref<PathListItem>& aItem, const char* aText) { return (aInstance->*aMethod)(aItem, aText); };
		return this;
	}

	template<typename InstanceType>
	PathTableRow* PathTableRow::OnRenameTextCommitted(InstanceType* aInstance, void(InstanceType::* aMethod)(const Ref<PathListItem>&, const char*, ETextCommitType)) noexcept
	{
		OnRenameTextCommittedCallback = [aInstance, aMethod](const Ref<PathListItem>& aItem, const char* aText, ETextCommitType aTextCommitType) { return (aInstance->*aMethod)(aItem, aText, aTextCommitType); };
		return this;
	}

}