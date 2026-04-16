#include "DragDropOperation.h"

#include "UI/Widgets/HorizontalBox.h"

namespace Relentless
{
	DragDropOperationBase::~DragDropOperationBase() noexcept = default;

	void DragDropOperationBase::CreatePreview() noexcept
	{
		m_pPreviewWidget = RLS_NEW HorizontalBox();
	}

	const Ref<IBaseWidget> DragDropOperationBase::GetPreview() const noexcept
	{
		return m_pPreviewWidget;
	}

}
