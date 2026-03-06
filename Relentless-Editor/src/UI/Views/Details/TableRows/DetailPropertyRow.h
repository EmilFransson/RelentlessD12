#pragma once
#include <Relentless.h>

#include "UI/Widgets/IWidget.h"
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	class DetailPropertyRow : public ITableRow
	{
	public:
		DetailPropertyRow() noexcept;
		virtual ~DetailPropertyRow() noexcept override;

		const Color& GetBackgroundColor() const noexcept override;
		uint32 GetNumColumns() noexcept override;

		bool IsDragDropEligible() noexcept override;

		void OnRenderColumn(uint32 aColumn) noexcept override;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		void SetNameContent(Ref<IBaseWidget> aWidget) noexcept;
		void SetValueContent(Ref<IBaseWidget> aWidget) noexcept;
		void SetResetContent(Ref<IBaseWidget> aWidget) noexcept;

		Broadcaster<void()> OnDestroy;
	private:
		std::vector<Ref<IBaseWidget>> m_ColumnWidgets2;
	};
}