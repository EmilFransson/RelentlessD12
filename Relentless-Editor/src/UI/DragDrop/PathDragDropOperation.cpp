#include "PathDragDropOperation.h"

#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"

namespace Relentless
{
	PathDragDropOperation::PathDragDropOperation(const std::vector<String>& somePaths, const String& aPreviewText) noexcept
		: m_Paths{somePaths},
		  m_PreviewText{aPreviewText}
	{
	}

	void PathDragDropOperation::CreatePreview() noexcept
	{
		Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
		pBox->SetPadding({ 5.0f, 5.0f, 5.0f, 5.0f });
		pBox->SetSpacing(5.0f);

		m_pSymbolLabel = pBox->AddWidget(RLS_NEW Label(ICON_FA_CHECK));
		m_pSymbolLabel->SetTextColor(Colors::Green);
		m_pSymbolLabel->SetIsVisible(false);
		m_pSymbolLabel->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
			
		m_pPreviewLabel = pBox->AddWidget(RLS_NEW Label(m_PreviewText));
		m_pPreviewLabel->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pPreviewWidget = pBox;
	}

	const std::vector<String>& PathDragDropOperation::GetPaths() const noexcept
	{
		return m_Paths;
	}

	const String& PathDragDropOperation::GetPrimaryDraggedPath() const noexcept
	{
		return m_Paths.back();
	}

	uint32 PathDragDropOperation::GetNumDraggedPaths() const noexcept
	{
		return static_cast<uint32>(m_Paths.size());
	}

	void PathDragDropOperation::SetDrawSymbolLabel(bool aState) noexcept
	{
		m_pSymbolLabel->SetIsVisible(aState);
	}

	void PathDragDropOperation::SetPreviewText(const String& aPreviewText) noexcept
	{
		m_pPreviewLabel->SetText(aPreviewText);
	}

	void PathDragDropOperation::SetSymbol(const String& aSymbolLabel, const Color& aColor) noexcept
	{
		m_pSymbolLabel->SetText(aSymbolLabel);
		m_pSymbolLabel->SetTextColor(aColor);
	}
}